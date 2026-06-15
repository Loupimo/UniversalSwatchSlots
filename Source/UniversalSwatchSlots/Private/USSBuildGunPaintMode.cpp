#include "USSBuildGunPaintMode.h"

#include "UniversalSwatchSlots.h"            // LogUSS
#include "USSModeDescriptor.h"
#include "USSSameSwatchPaintMode.h"
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
#include "FGFactoryColoringTypes.h"           // FFactoryCustomizationData::Initialize hook
#include "FGCustomizationRecipe.h"
#include "FGLightweightBuildableSubsystem.h"
#include "FGInventoryComponent.h"

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
TSet<TWeakObjectPtr<AFGBuildable>> FUSSBuildGunPaintMode::PreviewedBuildables;
TWeakObjectPtr<UClass> FUSSBuildGunPaintMode::LastPreviewDescClass;
uint8 FUSSBuildGunPaintMode::LastPreviewPatternRotation = 0;
TWeakObjectPtr<AFGBuildable> FUSSBuildGunPaintMode::LastGameFocusedBuildable;

// True if the player can pay 'cost' from 'inventory' (empty cost / free build is always affordable).
static bool USSCanAffordCost(UFGInventoryComponent* inventory, const TArray<FItemAmount>& cost)
{
	if (cost.Num() == 0)
	{
		return true; // nothing to pay
	}
	if (!inventory)
	{
		return false; // it costs something but we have nowhere to pay from
	}
	for (const FItemAmount& item : cost)
	{
		if (item.ItemClass && item.Amount > 0 && !inventory->HasItems(item.ItemClass, item.Amount))
		{
			return false;
		}
	}
	return true;
}

// Removes 'cost' from 'inventory' (assumes affordability was already checked).
static void USSDeductCost(UFGInventoryComponent* inventory, const TArray<FItemAmount>& cost)
{
	if (!inventory)
	{
		return;
	}
	for (const FItemAmount& item : cost)
	{
		if (item.ItemClass && item.Amount > 0)
		{
			inventory->Remove(item.ItemClass, item.Amount);
		}
	}
}

int32 FUSSBuildGunPaintMode::CountPlanBuildings(AFGBlueprintProxy* proxy)
{
	if (!proxy)
	{
		return 0;
	}

	TArray<AFGBuildable*> buildables;
	proxy->CollectBuildables(buildables);
	int32 count = buildables.Num();

	for (const FBuildableClassLightweightIndices& entry : proxy->GetLightweightClassAndIndices())
	{
		count += entry.Indices.Num();
	}
	return count;
}

TArray<FItemAmount> FUSSBuildGunPaintMode::ScaleCost(const TArray<FItemAmount>& perApplicationCost, int32 buildingCount)
{
	const int32 scale = FMath::Max(buildingCount, 0);

	TArray<FItemAmount> scaled;
	scaled.Reserve(perApplicationCost.Num());
	for (const FItemAmount& item : perApplicationCost)
	{
		scaled.Add(FItemAmount(item.ItemClass, item.Amount * scale));
	}
	return scaled;
}

AFGBuildable* FUSSBuildGunPaintMode::ResolveAimedBuildable(UFGBuildGunStatePaint* self, AActor* hitActor)
{
	// Swatch / pattern / skin: the hit actor is the real building and carries the proxy.
	if (AFGBuildable* hit = Cast<AFGBuildable>(hitActor))
	{
		if (hit->GetBlueprintProxy())
		{
			return hit;
		}
	}

	// Material: the engine aims a transient mesh-swap preview actor (no proxy). The real aimed
	// building is kept by the paint state itself (private, friended via AccessTransformers.ini).
	// We accept the first candidate that actually belongs to a blueprint.
	if (self)
	{
		if (AFGBuildable* aimed = Cast<AFGBuildable>(self->mCurrentlyAimedAtActor))
		{
			if (aimed->GetBlueprintProxy())
			{
				return aimed;
			}
		}
		if (AFGBuildable* target = Cast<AFGBuildable>(self->mCurrentCustomizationTarget))
		{
			if (target->GetBlueprintProxy())
			{
				return target;
			}
		}
	}

	// Last resort: whatever the hit actor was (may be null / proxy-less -> no propagation).
	return Cast<AFGBuildable>(hitActor);
}

void FUSSBuildGunPaintMode::ApplyToBlueprintPlan(UFGBuildGunStatePaint* self, FFactoryCustomizationData* customizationData, AFGBlueprintProxy* proxy, AFGBuildable* skipBuildable)
{
	if (!self || !proxy)
	{
		return; // the aimed building isn't part of a blueprint
	}

	// Actor buildables (machines, etc.).
	TArray<AFGBuildable*> buildables;
	proxy->CollectBuildables(buildables);
	for (AFGBuildable* buildable : buildables)
	{
		if (buildable && buildable != skipBuildable)
		{
			buildable->SetCustomizationData_Native(*customizationData); // skipCombine=false -> merges with existing
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
					tempInfo->Buildable->SetCustomizationData_Native(*customizationData); // skipCombine=false -> merges
					lightweightSubsystem->CopyCustomizationDataFromTemporaryToInstance(tempInfo->Buildable);
				}
				else
				{
					// Fallback: at least update the persistent data so it saves.
					lightweightSubsystem->SetCustomizationDataOnInstance(entry.BuildableClass, *customizationData, index);
				}
			}
		}
	}
}

void FUSSBuildGunPaintMode::ApplyMaterialSwapToPlan(UFGBuildGunStatePaint* self, AFGBlueprintProxy* proxy, TSubclassOf<UFGFactoryCustomizationDescriptor_Material> material)
{
	if (!self || !proxy || !material)
	{
		return;
	}

	AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(self->GetWorld());
	if (!lightweightSubsystem)
	{
		return;
	}

	// The material maps each source buildable class -> the recipe (and thus mesh) to switch to.
	UFGFactoryCustomizationDescriptor_Material* materialCDO = material.GetDefaultObject();
	if (!materialCDO)
	{
		return;
	}
	const TMap<TSubclassOf<AFGBuildable>, TSubclassOf<UFGRecipe>>& buildableMap = materialCDO->GetBuildableMap();

	// Snapshot the plan's lightweight (class, index) pairs BEFORE mutating the subsystem: removing
	// and re-adding instances edits the proxy's index lists, which we'd otherwise be iterating.
	// We run instead of (not after) the game's paint, so the proxy still includes the focused
	// instance -> it gets swapped like all the others and stays linked to the plan.
	struct FPlanInstance { TSubclassOf<AFGBuildable> Class; int32 Index; };
	TArray<FPlanInstance> planInstances;
	for (const FBuildableClassLightweightIndices& entry : proxy->GetLightweightClassAndIndices())
	{
		for (int32 index : entry.Indices)
		{
			planInstances.Add({ entry.BuildableClass, index });
		}
	}

	for (const FPlanInstance& instance : planInstances)
	{
		const TSubclassOf<UFGRecipe> newRecipe = buildableMap.FindRef(instance.Class);
		if (!newRecipe)
		{
			continue; // this material doesn't change this buildable type
		}
		const TSubclassOf<AFGBuildable> newClass = AFGBuildable::GetBuildableClassFromRecipe(newRecipe);
		if (!newClass || newClass == instance.Class)
		{
			continue; // no mesh/class change needed
		}

		FRuntimeBuildableInstanceData* runtimeData =
			lightweightSubsystem->GetRuntimeDataForBuildableClassAndIndex(instance.Class, instance.Index);
		if (!runtimeData)
		{
			continue;
		}

		// Fresh runtime data for the swapped class: keep transform / customization / proxy /
		// type-specific data + builder, set the new recipe and material. We deliberately do not
		// copy the runtime-only Handles (the new instance gets its own).
		FFactoryCustomizationData newCustomization = runtimeData->CustomizationData;
		newCustomization.MaterialDesc = material;

		FRuntimeBuildableInstanceData newData(runtimeData->Transform, newCustomization, newRecipe, runtimeData->BlueprintProxy);
		newData.TypeSpecificData = runtimeData->TypeSpecificData;
		newData.BuiltBy = runtimeData->BuiltBy;

		// Swap the instance (remove old class/index, add as the new class) and re-link it to the
		// blueprint so it stays part of the plan (the game's own apply leaves the focused one out).
		lightweightSubsystem->RemoveByInstanceIndex(instance.Class, instance.Index);
		const int32 newIndex = lightweightSubsystem->AddFromBuildableInstanceData(newClass, newData);

		proxy->UnregisterLightweightInstance(instance.Class, instance.Index);
		if (newIndex != INDEX_NONE)
		{
			proxy->RegisterLightweightInstance(newClass, newIndex);
		}
	}
}

void FUSSBuildGunPaintMode::RegisterHooks()
{
	UFGBuildGunStatePaint* paintCDO = GetMutableDefault<UFGBuildGunStatePaint>();

	// (f) Crash-safety for joining clients / unpopulated colour slots. A building resolves its colour in
	//     FFactoryCustomizationData::Initialize(gameState), which calls
	//     gameState->GetBuildingColorDataForSlot(slot) -> indexes mBuildingColorSlots_Data. This runs
	//     very early (AFGBuildable::BeginPlay, the buildable subsystem tick) and can run before the
	//     game state / its colour-slot array is ready: a NULL game state dereferences a null 'this'
	//     (reads the array field off ~0x668 -> crash), and a valid-but-empty array dereferences a null
	//     data pointer. We hook Initialize and skip it while the colour slots aren't ready; the
	//     buildable subsystem re-inits the building once they are (IsInitialized stays false meanwhile).
	//
	//     IMPORTANT: we hook Initialize (returns void) rather than GetBuildingColorDataForSlot (returns
	//     FFactoryCustomizationColorSlot BY VALUE). SML's by-value-return hook (ApplyCallUserTypeByValue)
	//     inserts a hidden return pointer and assumes the MSVC argument order (this, then return-ptr).
	//     On the Linux dedicated server (System V AMD64 ABI) a 40-byte struct return swaps that order
	//     (sret pointer first, then this), so the trampoline corrupts the call -- crashing
	//     AFGBuildableHologram::OnRep_CustomizationData even for normal in-range lookups (e.g. placing
	//     the HUB). A void hook has no by-value return, so it is ABI-identical on every platform, and it
	//     only intervenes while the slots are unavailable -- the server's build/paint paths run untouched.
	SUBSCRIBE_METHOD(FFactoryCustomizationData::Initialize,
		[](auto& scope, FFactoryCustomizationData* self, AFGGameState* gameState, int32 forceDataSize)
		{
			if (!gameState || gameState->mBuildingColorSlots_Data.Num() == 0)
			{
				// Colour slots not ready yet (null game state on a joining client, or the array hasn't
				// been set up / replicated). Skip init so the original never dereferences a null 'this'
				// or an empty array in GetBuildingColorDataForSlot; the buildable subsystem re-runs this
				// once the slots exist (the colours are then applied by the load-time re-apply).
				scope.Cancel();
				return;
			}
			// Game state and colour slots are available -> run the original untouched.
		});
	UE_LOG(LogUSS, Verbose, TEXT("Registered FFactoryCustomizationData::Initialize colour-slot safety hook."));

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
				out_buildModes.AddUnique(UUSSPaintSameSwatchModeDescriptor::StaticClass());
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
				FUSSSameSwatchPaintMode::OnEndPaintState(state);
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
				UpdateBlueprintHighlight(state);            // Blueprint mode (clears if inactive)
				FUSSSameSwatchPaintMode::UpdateHighlight(state); // Same-Swatch mode (clears if inactive)
			}
		});

	// (e) Blueprint paint mode: when our Blueprint mode is active, painting one building
	//     of a blueprint applies the same customization to every building of that
	//     blueprint. Runs server-side (Server_ExecutePaint_Implementation) after the
	//     normal single-target paint.
	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::Server_ExecutePaint_Implementation, paintCDO,
		[](auto& scope, UFGBuildGunStatePaint* self, uint8 mode, FFactoryCustomizationData customizationData, AActor* hitActor)
		{
			// Resolve the aimed plan BEFORE anything runs. In material mode the original aims a
			// transient mesh-swap preview (so hitActor has no blueprint proxy), so we resolve the
			// real aimed building and its proxy up front.
			AFGBuildGun* gun = self ? self->GetBuildGun() : nullptr;

			// "Same Swatch" mode: paint only the plan elements that share the aimed element's swatch.
			// The reference swatch is resolved server-side from the aimed element (hitActor) -- the same
			// way Blueprint mode resolves the plan -- so this is robust on dedicated servers without
			// shipping the client's lock state across. The client's locked-fire guard ensures that,
			// while locked, the player can only fire at a locked-swatch element (so hitActor's swatch
			// is the locked one).
			if (FUSSSameSwatchPaintMode::IsModeActive(gun))
			{
				AFGBuildable* aimed = ResolveAimedBuildable(self, hitActor);
				AFGBlueprintProxy* ssProxy = aimed ? aimed->GetBlueprintProxy() : nullptr;
				UClass* refSwatch = aimed ? FUSSSameSwatchPaintMode::GetBuildableSwatchClass(aimed) : nullptr;
				if (!ssProxy || !refSwatch)
				{
					scope(self, mode, customizationData, hitActor); // not a plan / no swatch -> vanilla single
					return;
				}
				scope.Cancel();
				FUSSSameSwatchPaintMode::ApplyToPlanFiltered(self, &customizationData, ssProxy, refSwatch);
				return;
			}

			const bool blueprintMode = gun && gun->GetCurrentBuildGunMode() == UUSSModeDescriptor::StaticClass();

			AFGBuildable* skipBuildable = blueprintMode ? ResolveAimedBuildable(self, hitActor) : nullptr;
			AFGBlueprintProxy* proxy = skipBuildable ? skipBuildable->GetBlueprintProxy() : nullptr;

			// Not our blueprint paint (or not aiming at a plan): vanilla single-target paint.
			if (!blueprintMode || !proxy)
			{
				scope(self, mode, customizationData, hitActor);
				return;
			}

			// What is the player painting right now? The active recipe's descriptor tells us
			// (swatch / pattern / material / skin). A material is a recipe/mesh swap, not
			// per-instance data, so it needs a completely different path.
			TSubclassOf<UFGFactoryCustomizationDescriptor_Material> material = nullptr;
			if (const TSubclassOf<UFGCustomizationRecipe> activeRecipe = self->GetActiveRecipe())
			{
				UClass* activeDescClass = *UFGCustomizationRecipe::GetCustomizationDescriptor(activeRecipe);
				if (activeDescClass && activeDescClass->IsChildOf(UFGFactoryCustomizationDescriptor_Material::StaticClass()))
				{
					material = TSubclassOf<UFGFactoryCustomizationDescriptor_Material>(activeDescClass);
				}
			}

			// Vanilla cost: painting the whole plan costs the per-building customization cost times
			// the number of buildings. Block the entire paint up front if the player can't afford it
			// (unless no-build-cost is on), so we never paint a partial plan.
			AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(gun->GetInstigator());
			UFGInventoryComponent* inventory = character ? character->GetInventory() : nullptr;
			const bool bFreeBuild = true;// inventory&& inventory->GetNoBuildCost();
			const int32 buildingCount = CountPlanBuildings(proxy);
			const TArray<FItemAmount> perApplyCost = self->GetCustomizationCost();

			if (!bFreeBuild && !USSCanAffordCost(inventory, ScaleCost(perApplyCost, buildingCount)))
			{
				scope.Cancel();                  // paint nothing
				self->OnApplyCustomizationFailed();
				return;
			}

			if (material)
			{
				// We swap EVERY plan lightweight ourselves, including the focused one, so they all
				// keep their existing customization and stay linked to the blueprint. Suppress the
				// original paint (SML auto-forwards unless cancelled): it would reconstruct only the
				// focused building, drop it from the plan and lose its prior customization.
				scope.Cancel();
				if (!bFreeBuild)
				{
					USSDeductCost(inventory, ScaleCost(perApplyCost, buildingCount)); // charge all N
				}
				ApplyMaterialSwapToPlan(self, proxy, material);
			}
			else
			{
				// The game paints + charges the focused building (1); we paint the rest and charge
				// the remaining (N-1) so the total is exactly the whole-plan cost.
				scope(self, mode, customizationData, hitActor);
				if (!bFreeBuild)
				{
					USSDeductCost(inventory, ScaleCost(perApplyCost, buildingCount - 1));
				}
				ApplyToBlueprintPlan(self, &customizationData, proxy, skipBuildable);
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

	// What the player is about to paint = the active customization recipe's descriptor (a swatch,
	// pattern, material or skin). Drives the visual-only live preview on every plan building.
	UFGBuildGunStatePaint* paintStateTyped = Cast<UFGBuildGunStatePaint>(paintState);
	const TSubclassOf<UFGCustomizationRecipe> activeRecipe = paintStateTyped ? paintStateTyped->GetActiveRecipe() : nullptr;
	UClass* activeDescClass = activeRecipe ? *UFGCustomizationRecipe::GetCustomizationDescriptor(activeRecipe) : nullptr;
	UWorld* world = paintState->GetWorld();
	AFGGameState* gameState = world ? world->GetGameState<AFGGameState>() : nullptr;

	// The per-instance colour preview writes the engine's instanced-mesh buffers
	// (ApplyCustomizationData_Native -> UFGColoredInstanceManager). On a network client those same
	// buffers are written concurrently by lightweight replication, and the two writes race ->
	// FInstanceSceneDataBuffers::BeginWriteAccess assert (hard crash). So we keep the (safe)
	// custom-depth outline on every plan building, but only apply the colour preview when we are
	// the authority (host / single player) -- unless the player opts in via the mod's world-module
	// toggle (GetClientPreview). The on-click result is identical either way.
	const bool bCanPreviewColor = (world && world->GetNetMode() != NM_Client) || (world && GetWorldModule(world)->GetClientPreview());

	// Writing per-instance colour data on instanced meshes is racy (it can collide with the render
	// thread / lightweight replication, especially on clients). So we apply the colour preview as
	// rarely as possible: ONCE per building, and again only when the active customization changes
	// (descriptor or pattern rotation). The outline below is cheap/safe (custom depth) and stays
	// per-tick. PreviewedBuildables tracks which buildings already carry the current preview.
	const uint8 currentPatternRotation = paintStateTyped ? paintStateTyped->mPatternRotation : 0;
	if (LastPreviewDescClass.Get() != activeDescClass || LastPreviewPatternRotation != currentPatternRotation)
	{
		PreviewedBuildables.Reset(); // customization changed -> re-apply once to every plan building
		LastPreviewDescClass = activeDescClass;
		LastPreviewPatternRotation = currentPatternRotation;
	}

	// The game previews the active customization on the building you aim at and REVERTS it when you
	// move off (restoring its real look), which wipes our colour preview on that building. Detect
	// the game's focus change and evict the building you just left so the loop below re-applies the
	// preview to it once.
	AFGBuildable* gameFocus = paintStateTyped ? Cast<AFGBuildable>(paintStateTyped->mCurrentlyAimedAtActor) : nullptr;
	if (LastGameFocusedBuildable.Get() != gameFocus)
	{
		if (AFGBuildable* leftBuildable = LastGameFocusedBuildable.Get())
		{
			PreviewedBuildables.Remove(TWeakObjectPtr<AFGBuildable>(leftBuildable));
		}
		LastGameFocusedBuildable = gameFocus;
	}

	for (const TWeakObjectPtr<AFGBuildable>& weakBuildable : HighlightedBuildables)
	{
		AFGBuildable* buildable = weakBuildable.Get();
		if (!IsValid(buildable))
		{
			continue;
		}

		// Outline is safe to refresh every tick (the game clears it when the cursor leaves a building).
		IFGColorInterface::Execute_StartIsAimedAtForColor(buildable, character, true);

		// Colour preview only the first time we see this building with the current customization
		// (visual-only: ApplyCustomizationData_Native doesn't touch the saved data -> revertible).
		if (bCanPreviewColor && activeDescClass && gameState && !PreviewedBuildables.Contains(weakBuildable))
		{
			FFactoryCustomizationData previewData = buildable->GetCustomizationData_Native(); // copy real data
			if (activeDescClass->IsChildOf(UFGFactoryCustomizationDescriptor_Swatch::StaticClass()))
			{
				previewData.SwatchDesc = activeDescClass;
			}
			else if (activeDescClass->IsChildOf(UFGFactoryCustomizationDescriptor_Pattern::StaticClass()))
			{
				previewData.PatternDesc = activeDescClass;
				// Pattern rotation lives in the paint state's private mPatternRotation, aligned
				// per-building (so patterns don't spin with foundation orientation), reachable via
				// the Access Transformer friend. Rotating re-applies (it changes the signature above).
				previewData.PatternRotation =
					paintStateTyped->AlignPatternRotationWithActor(buildable, paintStateTyped->mPatternRotation);
			}
			else if (activeDescClass->IsChildOf(UFGFactoryCustomizationDescriptor_Material::StaticClass()))
			{
				previewData.MaterialDesc = activeDescClass;
			}
			else if (activeDescClass->IsChildOf(UFGFactoryCustomizationDescriptor_Skin::StaticClass()))
			{
				previewData.SkinDesc = activeDescClass;
			}
			previewData.Initialize(gameState);
			buildable->ApplyCustomizationData_Native(previewData); // visual only
			PreviewedBuildables.Add(weakBuildable);
		}
	}
}

void FUSSBuildGunPaintMode::ClearBlueprintHighlight()
{
	AFGCharacterPlayer* character = HighlightCharacter.Get();

	for (const TWeakObjectPtr<AFGBuildable>& weakBuildable : HighlightedBuildables)
	{
		AFGBuildable* buildable = weakBuildable.Get();
		if (!IsValid(buildable))
		{
			continue;
		}

		if (character)
		{
			IFGColorInterface::Execute_StopIsAimedAtForColor(buildable, character);
		}

		// Revert ONLY the buildings we actually previewed (tracked in PreviewedBuildables): restores
		// their real look, and keeps the racy colour writes to the same minimum as the apply.
		// (Lightweight temps also revert on their own when the subsystem reclaims them.)
		if (PreviewedBuildables.Contains(weakBuildable))
		{
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
	PreviewedBuildables.Reset();
	LastPreviewDescClass = nullptr;
	LastPreviewPatternRotation = 0;
	LastGameFocusedBuildable = nullptr;
	LightweightInstigator = nullptr;
	HighlightedProxy = nullptr;
	HighlightCharacter = nullptr;
}
