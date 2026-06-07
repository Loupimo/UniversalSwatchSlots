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
#include "FGLightweightBuildableSubsystem.h"
#include "FGBlueprintFunctionLibrary.h" // EOutlineColor

#include "Patching/NativeHookManager.h"
#include "Module/WorldModuleManager.h"

#include "AbstractInstanceInterface.h"
#include "InstanceData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

TWeakObjectPtr<UUSSPaintModeWidget> FUSSBuildGunPaintMode::IndicatorWidget;
TWeakObjectPtr<AFGBlueprintProxy> FUSSBuildGunPaintMode::HighlightedProxy;
TArray<TWeakObjectPtr<AFGBuildable>> FUSSBuildGunPaintMode::HighlightedBuildables;
TWeakObjectPtr<AFGCharacterPlayer> FUSSBuildGunPaintMode::HighlightCharacter;
TWeakObjectPtr<AActor> FUSSBuildGunPaintMode::HighlightISMHolder;

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
						// Authoritative data + save. Live replication to connected clients on a
						// dedicated server is handled by the game's bundle system (lags for big
						// batches); the data is correct so it shows on reconnect / for joiners.
						lightweightSubsystem->SetCustomizationDataOnInstance(entry.BuildableClass, customizationData, index);
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

	// Rebuild the highlighted set only when the aimed blueprint changes.
	if (targetProxy != HighlightedProxy.Get())
	{
		ClearBlueprintHighlight(); // remove the previous blueprint's outline

		if (targetProxy)
		{
			// Per-building outline for actor buildables (machines, etc.).
			TArray<AFGBuildable*> buildables;
			targetProxy->CollectBuildables(buildables);
			for (AFGBuildable* buildable : buildables)
			{
				if (buildable)
				{
					HighlightedBuildables.Add(buildable);
				}
			}

			// Lightweight instances (foundations, walls, ...) have no actor, so we render
			// custom-depth outline copies of their meshes on a holder actor, with the same
			// stencil as the machines' outline -> identical look. Render-in-main-pass is off
			// so only the outline shows (no duplicate mesh). No temp buildables are spawned,
			// so the lightweight subsystem is left untouched.
			if (UWorld* world = paintState->GetWorld())
			{
				AActor* holder = nullptr;
				TMap<UStaticMesh*, UInstancedStaticMeshComponent*> meshToISM;
				AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(world);

				for (const FBuildableClassLightweightIndices& entry : targetProxy->GetLightweightClassAndIndices())
				{
					AFGBuildable* cdo = entry.BuildableClass.GetDefaultObject();
					if (!cdo || !lightweightSubsystem)
					{
						continue;
					}

					const TArray<FInstanceData> instanceMeshes = IAbstractInstanceInterface::Execute_GetActorLightweightInstanceData(cdo);

					for (int32 index : entry.Indices)
					{
						FLightweightBuildableInstanceRef ref;
						ref.Initialize(lightweightSubsystem, entry.BuildableClass, index);
						const FTransform instanceWorld = ref.GetBuildableTransform();

						for (const FInstanceData& meshData : instanceMeshes)
						{
							if (!meshData.StaticMesh)
							{
								continue;
							}

							if (!holder)
							{
								holder = world->SpawnActor<AActor>();
								USceneComponent* root = NewObject<USceneComponent>(holder, TEXT("Root"));
								holder->SetRootComponent(root);
								root->RegisterComponent();
							}

							UInstancedStaticMeshComponent*& ism = meshToISM.FindOrAdd(meshData.StaticMesh);
							if (!ism)
							{
								ism = NewObject<UInstancedStaticMeshComponent>(holder);
								ism->SetStaticMesh(meshData.StaticMesh);
								ism->SetMobility(EComponentMobility::Movable);
								ism->SetCollisionEnabled(ECollisionEnabled::NoCollision);
								ism->SetCastShadow(false);
								ism->SetRenderCustomDepth(true);
								ism->SetCustomDepthStencilValue((int32)EOutlineColor::OC_HOLOGRAMLINE);
								ism->RegisterComponent();
								ism->AttachToComponent(holder->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
								// Render only the custom-depth outline, not the (uncolored, black) mesh.
								// Setting this AFTER registration + dirtying the render state makes it
								// take effect (setting it before RegisterComponent did not).
								ism->SetRenderInMainPass(false);
								ism->MarkRenderStateDirty();
							}

							// Small shrink as a safety net in case the mesh still renders in the main
							// pass (keeps any black hidden inside the real building). If SetRenderInMainPass
							// works, only the outline shows and this just insets it very slightly.
							// Tweak toward 1.0 for a crisper outline, toward 0.98 if any black peeks.
							FTransform outlineTransform = meshData.RelativeTransform * instanceWorld;
							outlineTransform.SetScale3D(outlineTransform.GetScale3D() * 0.99f);
							ism->AddInstance(outlineTransform, /*bWorldSpace=*/true);
						}
					}
				}

				HighlightISMHolder = holder;
			}

			HighlightedProxy = targetProxy;
			HighlightCharacter = character;
		}
	}

	// Re-apply every frame: the game hides a building's outline when the cursor leaves
	// it, which would clear our preview when moving within the same blueprint. We run
	// after the game's tick (scope already ran), so re-showing it wins.
	if (HighlightedProxy.Get())
	{
		for (const TWeakObjectPtr<AFGBuildable>& weakBuildable : HighlightedBuildables)
		{
			if (AFGBuildable* buildable = weakBuildable.Get())
			{
				IFGColorInterface::Execute_StartIsAimedAtForColor(buildable, character, true);
			}
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
		}
	}

	// Destroy the lightweight outline ISM holder, if any.
	if (AActor* holder = HighlightISMHolder.Get())
	{
		holder->Destroy();
	}

	HighlightedBuildables.Reset();
	HighlightISMHolder = nullptr;
	HighlightedProxy = nullptr;
	HighlightCharacter = nullptr;
}
