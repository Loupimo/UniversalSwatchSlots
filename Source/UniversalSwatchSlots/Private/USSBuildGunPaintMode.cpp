#include "USSBuildGunPaintMode.h"

#include "UniversalSwatchSlots.h"            // LogUSS
#include "USSModeDescriptor.h"
#include "USSPaintModeWidget.h"
#include "UniversalSwatchSlotsWorldModule.h" // PaintModeWidgetClass

#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunPaint.h"
#include "FGBuildGunModeDescriptor.h"
#include "Input/FGEnhancedInputComponent.h"
#include "Buildables/FGBuildable.h"
#include "FGBlueprintProxy.h"
#include "FGColorInterface.h"
#include "FGCharacterPlayer.h"
#include "FGGameState.h"
#include "FGLightweightBuildableSubsystem.h"

#include "Patching/NativeHookManager.h"
#include "Module/WorldModuleManager.h"

#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

TWeakObjectPtr<UUSSPaintModeWidget> FUSSBuildGunPaintMode::IndicatorWidget;
TWeakObjectPtr<AFGBlueprintProxy> FUSSBuildGunPaintMode::HighlightedProxy;
TArray<TWeakObjectPtr<AFGBuildable>> FUSSBuildGunPaintMode::HighlightedBuildables;
TWeakObjectPtr<AFGCharacterPlayer> FUSSBuildGunPaintMode::HighlightCharacter;
TWeakObjectPtr<AActor> FUSSBuildGunPaintMode::LightweightInstigator;

void FUSSBuildGunPaintMode::RegisterHooks()
{
	UFGBuildGunStatePaint* paintCDO = GetMutableDefault<UFGBuildGunStatePaint>();

	// (a) Expose two build modes (Default + Blueprint) while in the Paint state,
	//     so the build gun shows a "Default / Blueprint" roller like Dismantle.
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::GetSupportedBuildModes, paintCDO,
		[](auto& scope, const UFGBuildGunState* state, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildModes)
		{
			// Run the game's implementation first so we append instead of getting overwritten.
			scope(state, out_buildModes);

			// This stub is shared by every state, so only add our modes for Paint.
			if (state && state->IsA<UFGBuildGunStatePaint>())
			{
				out_buildModes.AddUnique(UUSSPaintModeDefault::StaticClass());
				out_buildModes.AddUnique(UUSSModeDescriptor::StaticClass());
			}
		});

	// (b) Start in "Default" when entering Paint so the selector has a valid current mode.
	// C4191: SML's NativeHookManager intentionally reinterpret_casts when hooking a
	// function that returns a class type by value (here TSubclassOf<...>). The warning
	// is emitted from SML at this instantiation point, so we silence it locally (MSVC).
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4191)
#endif
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::GetInitialBuildGunMode, paintCDO,
		[](auto& scope, const UFGBuildGunState* state)
		{
			if (state && state->IsA<UFGBuildGunStatePaint>())
			{
				scope.Override(TSubclassOf<UFGBuildGunModeDescriptor>(UUSSPaintModeDefault::StaticClass()));
			}
			// Other states: leave untouched, the original is auto-forwarded.
		});
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

	// (c) Cycling on the mode-select key (tap) and the radial (hold) are owned by the
	//     UUSSPaintModeWidget, which binds the ModeSelect input itself while in Paint.

	// (d) HUD widget: show while the Paint state is active, hide when it ends.
	SUBSCRIBE_METHOD(UFGBuildGunStatePaint::BeginState,
		[](auto& scope, UFGBuildGunState* state)
		{
			scope(state); // run the game's BeginState first
			if (state && state->IsA<UFGBuildGunStatePaint>())
			{
				ShowIndicator(state);
			}
		});

	SUBSCRIBE_METHOD(UFGBuildGunStatePaint::EndState,
		[](auto& scope, UFGBuildGunState* state)
		{
			scope(state);
			if (state && state->IsA<UFGBuildGunStatePaint>())
			{
				HideIndicator();
				ClearBlueprintHighlight();
			}
		});

	// (e2) Preview highlight: while aiming at a blueprint in Blueprint mode, outline
	//      every building of that blueprint so the player sees what will be painted.
	SUBSCRIBE_METHOD(UFGBuildGunStatePaint::TickState,
		[](auto& scope, UFGBuildGunState* state, float deltaTime)
		{
			scope(state, deltaTime);
			if (state && state->IsA<UFGBuildGunStatePaint>())
			{
				UpdateBlueprintHighlight(state);
			}
		});

	// (e) Blueprint paint mode: when our Blueprint mode is active, painting one building
	//     of a blueprint applies the same customization to every building of that
	//     blueprint. Runs server-side (Server_ExecutePaint_Implementation) after the
	//     normal single-target paint.
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::Server_ExecutePaint_Implementation, paintCDO,
		[](auto& scope, UFGBuildGunStatePaint* self, uint8 mode, FFactoryCustomizationData customizationData, AActor* hitActor)
		{
			scope(self, mode, customizationData, hitActor); // normal single-target paint first

			AFGBuildGun* gun = self ? self->GetBuildGun() : nullptr;
			if (!gun || gun->GetCurrentBuildGunMode() != UUSSModeDescriptor::StaticClass())
			{
				return; // only expand when our Blueprint paint mode is active
			}

			AFGBuildable* hitBuildable = Cast<AFGBuildable>(hitActor);
			AFGBlueprintProxy* proxy = hitBuildable ? hitBuildable->GetBlueprintProxy() : nullptr;
			if (!proxy)
			{
				return; // the aimed building isn't part of a blueprint
			}

			// Actor buildables (machines, etc.).
			TArray<AFGBuildable*> buildables;
			proxy->CollectBuildables(buildables);
			for (AFGBuildable* buildable : buildables)
			{
				if (buildable && buildable != hitBuildable)
				{
					buildable->SetCustomizationData_Native(customizationData); // skipCombine=false -> merges with existing
					buildable->FlushNetDormancy(); // wake net dormancy so the change replicates live to connected clients
				}
			}

			// Lightweight instances (foundations, walls, ...) which have no actor and are
			// not returned by CollectBuildables. The proxy tracks them as class + indices.
			if (AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(self->GetWorld()))
			{
				for (const FBuildableClassLightweightIndices& entry : proxy->GetLightweightClassAndIndices())
				{
					for (int32 index : entry.Indices)
					{
						// Mirror the game's single-lightweight paint so it replicates live: get a
						// managed temporary actor for the instance, paint it, then copy the data back
						// to the instance via the game's own path. The subsystem owns the temp's
						// lifecycle (cleaned up end of tick) so we never Destroy it (no corruption).
						FRuntimeBuildableInstanceData* runtimeData =
							lightweightSubsystem->GetRuntimeDataForBuildableClassAndIndex(entry.BuildableClass, index);

						bool didSpawn = false;
						FInstanceToTemporaryBuildable* tempInfo = runtimeData
							? lightweightSubsystem->FindOrSpawnBuildableForRuntimeData(entry.BuildableClass, runtimeData, index, didSpawn)
							: nullptr;

						if (tempInfo && tempInfo->Buildable)
						{
							tempInfo->Buildable->SetCustomizationData_Native(customizationData); // skipCombine=false -> merges
							lightweightSubsystem->CopyCustomizationDataFromTemporaryToInstance(tempInfo->Buildable);
						}
						else
						{
							// Fallback: at least update the persistent data so it saves.
							lightweightSubsystem->SetCustomizationDataOnInstance(entry.BuildableClass, customizationData, index);
						}
					}
				}
			}
		});
}

UUniversalSwatchSlotsWorldModule* FUSSBuildGunPaintMode::GetWorldModule(UWorld* world)
{
	UWorldModuleManager* manager = world ? world->GetSubsystem<UWorldModuleManager>() : nullptr;
	return manager ? Cast<UUniversalSwatchSlotsWorldModule>(manager->FindModule(FName(TEXT("UniversalSwatchSlots")))) : nullptr;
}

void FUSSBuildGunPaintMode::ShowIndicator(UFGBuildGunState* paintState)
{
	if (!paintState)
	{
		return;
	}

	AFGBuildGun* gun = paintState->GetBuildGun();
	APawn* pawn = gun ? gun->GetInstigator() : nullptr;
	if (!pawn || !pawn->IsLocallyControlled())
	{
		return; // only the local player gets the HUD widget
	}

	APlayerController* pc = Cast<APlayerController>(pawn->GetController());
	if (!pc)
	{
		return;
	}

	UUniversalSwatchSlotsWorldModule* module = GetWorldModule(paintState->GetWorld());
	if (!module)
	{
		UE_LOG(LogUSS, Warning, TEXT("ShowIndicator: USS world module not found (is RootGameWorld_UniversalSwatchSlots based on UUniversalSwatchSlotsWorldModule?)"));
		return;
	}

	const TSubclassOf<UUSSPaintModeWidget> widgetClass = module->PaintModeWidgetClass;
	if (!widgetClass)
	{
		UE_LOG(LogUSS, Warning, TEXT("ShowIndicator: PaintModeWidgetClass is not set on RootGameWorld_UniversalSwatchSlots"));
		return;
	}

	HideIndicator();

	if (UUSSPaintModeWidget* widget = CreateWidget<UUSSPaintModeWidget>(pc, widgetClass))
	{
		// Set BuildGun + bind input BEFORE AddToViewport so it is already valid when the
		// widget's Event Construct runs. Setup() fires OnSetup() at the end.
		widget->Setup(gun, Cast<UFGEnhancedInputComponent>(pawn->InputComponent));
		widget->AddToViewport();
		IndicatorWidget = widget;
	}
}

void FUSSBuildGunPaintMode::HideIndicator()
{
	if (IndicatorWidget.IsValid())
	{
		IndicatorWidget->Teardown();
		IndicatorWidget->RemoveFromParent();
		IndicatorWidget.Reset();
	}
}

void FUSSBuildGunPaintMode::UpdateBlueprintHighlight(UFGBuildGunState* paintState)
{
	AFGBuildGun* gun = paintState ? paintState->GetBuildGun() : nullptr;
	AFGCharacterPlayer* character = gun ? Cast<AFGCharacterPlayer>(gun->GetInstigator()) : nullptr;

	if (!gun || !character || !character->IsLocallyControlled())
	{
		ClearBlueprintHighlight();
		return;
	}

	// Only consider a blueprint when our Blueprint paint mode is active.
	AFGBlueprintProxy* targetProxy = nullptr;
	if (gun->GetCurrentBuildGunMode() == UUSSModeDescriptor::StaticClass())
	{
		AFGBuildable* aimed = Cast<AFGBuildable>(gun->GetHitResult().GetActor());
		targetProxy = aimed ? aimed->GetBlueprintProxy() : nullptr;
	}

	// Rebuild the actor set only when the aimed blueprint changes.
	if (targetProxy != HighlightedProxy.Get())
	{
		ClearBlueprintHighlight();

		if (targetProxy)
		{
			// Actor buildables (machines, etc.) get the outline directly.
			TArray<AFGBuildable*> buildables;
			targetProxy->CollectBuildables(buildables);
			for (AFGBuildable* buildable : buildables)
			{
				if (buildable)
				{
					HighlightedBuildables.Add(buildable);
				}
			}

			// Keep the blueprint's lightweight instances spawned as temporary actors while we
			// highlight, via an instance-converter instigator covering the plan. Without it the
			// subsystem reclaims them between frames (and respawn is throttled), so the outline
			// would be incomplete. We remove the instigator on clear so they are reclaimed.
			if (AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(paintState->GetWorld()))
			{
				if (UBoxComponent* box = targetProxy->GetBoundingBox())
				{
					const float radius = box->GetScaledBoxExtent().Size();
					LightweightInstigator = lightweightSubsystem->AddInstanceConverterInstigator(
						radius, nullptr, FTransform(box->GetComponentLocation()));
				}
			}

			HighlightedProxy = targetProxy;
			HighlightCharacter = character;
		}
	}

	AFGBlueprintProxy* highlightedProxy = HighlightedProxy.Get();
	if (!highlightedProxy)
	{
		return;
	}

	// Lightweight instances (foundations, walls, ...) have no persistent actor. We spawn (and
	// keep alive each frame) managed temporary actors for them, then outline those temps exactly
	// like the game does for a single aimed lightweight (StartIsAimedAtForColor handles Nanite).
	// The subsystem owns the temps' lifecycle; once we stop referencing them (on clear) they are
	// reclaimed.
	if (AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(paintState->GetWorld()))
	{
		// Keep the instigator active each frame (the game's aim instigator follows the player),
		// otherwise the temps it should keep alive get reclaimed and respawned every tick.
		if (AActor* instigator = LightweightInstigator.Get())
		{
			if (UBoxComponent* box = highlightedProxy->GetBoundingBox())
			{
				lightweightSubsystem->SetInstanceInstigatorLocation(instigator, box->GetComponentLocation());
			}
		}

		for (const FBuildableClassLightweightIndices& entry : highlightedProxy->GetLightweightClassAndIndices())
		{
			for (int32 index : entry.Indices)
			{
				FRuntimeBuildableInstanceData* runtimeData =
					lightweightSubsystem->GetRuntimeDataForBuildableClassAndIndex(entry.BuildableClass, index);
				if (!runtimeData)
				{
					continue;
				}

				bool didSpawn = false;
				if (FInstanceToTemporaryBuildable* tempInfo =
					lightweightSubsystem->FindOrSpawnBuildableForRuntimeData(entry.BuildableClass, runtimeData, index, didSpawn))
				{
					if (tempInfo->Buildable)
					{
						HighlightedBuildables.AddUnique(tempInfo->Buildable);
					}
				}
			}
		}
	}

	// Active swatch (the color the player would paint) for the live preview below. Null if the
	// player has no swatch selected (e.g. painting a pattern/material) -> no color preview then.
	UFGBuildGunStatePaint* paintStateTyped = Cast<UFGBuildGunStatePaint>(paintState);
	const TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch> activeSwatch =
		paintStateTyped ? paintStateTyped->GetActiveSwatchDesc() : nullptr;
	AFGGameState* gameState = paintState->GetWorld() ? paintState->GetWorld()->GetGameState<AFGGameState>() : nullptr;

	// Re-apply every frame: the game clears a building's outline when the cursor leaves it, so we
	// re-show it (we run after the game's tick). We also apply the active swatch as a visual-only
	// preview (ApplyCustomizationData_Native doesn't touch the saved data, so we can revert it).
	for (const TWeakObjectPtr<AFGBuildable>& weakBuildable : HighlightedBuildables)
	{
		AFGBuildable* buildable = weakBuildable.Get();
		if (!buildable)
		{
			continue;
		}

		IFGColorInterface::Execute_StartIsAimedAtForColor(buildable, character, true);

		if (activeSwatch && gameState)
		{
			FFactoryCustomizationData previewData = buildable->GetCustomizationData_Native(); // copy real data
			previewData.SwatchDesc = activeSwatch;                                            // swap in the active color
			previewData.Initialize(gameState);
			buildable->ApplyCustomizationData_Native(previewData);                            // visual only
		}
	}
}

void FUSSBuildGunPaintMode::ClearBlueprintHighlight()
{
	AFGCharacterPlayer* character = HighlightCharacter.Get();
	for (const TWeakObjectPtr<AFGBuildable>& weakBuildable : HighlightedBuildables)
	{
		if (AFGBuildable* buildable = weakBuildable.Get())
		{
			if (character)
			{
				IFGColorInterface::Execute_StopIsAimedAtForColor(buildable, character);
			}

			// Revert the visual color preview to the building's real (untouched) customization data.
			// (Lightweight temps revert on their own when the subsystem reclaims them, but doing it
			// here too is harmless.)
			const FFactoryCustomizationData& realData = buildable->GetCustomizationData_Native();
			if (realData.IsInitialized())
			{
				buildable->ApplyCustomizationData_Native(realData);
			}
		}
	}

	// Remove the instigator so the subsystem reclaims the lightweight temporaries it kept alive.
	if (AActor* instigator = LightweightInstigator.Get())
	{
		if (UWorld* world = instigator->GetWorld())
		{
			if (AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(world))
			{
				lightweightSubsystem->RemoveInstanceConverterInstigator(instigator);
			}
		}
	}

	HighlightedBuildables.Reset();
	LightweightInstigator = nullptr;
	HighlightedProxy = nullptr;
	HighlightCharacter = nullptr;
}
