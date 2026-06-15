#include "USSSameSwatchPaintMode.h"

#include "UniversalSwatchSlots.h" // LogUSS
#include "USSModeDescriptor.h"
#include "USSBuildGunPaintMode.h"  // FUSSBuildGunPaintMode::ResolveAimedBuildable / GetWorldModule
#include "UniversalSwatchSlotsWorldModule.h" // OnSameSwatchInfoChanged event

#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunPaint.h"
#include "Buildables/FGBuildable.h"
#include "FGBlueprintProxy.h"
#include "FGColorInterface.h"
#include "FGCharacterPlayer.h"
#include "FGGameState.h"
#include "FGCustomizationRecipe.h"
#include "FGFactoryColoringTypes.h"
#include "FGLightweightBuildableSubsystem.h"

#include "Patching/NativeHookManager.h"
#include "Components/BoxComponent.h"

bool FUSSSameSwatchPaintMode::bLocked = false;
TWeakObjectPtr<UClass> FUSSSameSwatchPaintMode::LockedSwatch;
TWeakObjectPtr<AFGBlueprintProxy> FUSSSameSwatchPaintMode::LockedProxy;
TWeakObjectPtr<AFGBuildable> FUSSSameSwatchPaintMode::LockedFocus;
TWeakObjectPtr<UClass> FUSSSameSwatchPaintMode::LiveSwatch;
TWeakObjectPtr<AFGBlueprintProxy> FUSSSameSwatchPaintMode::LiveProxy;
TWeakObjectPtr<AFGBuildable> FUSSSameSwatchPaintMode::LiveFocus;
int32 FUSSSameSwatchPaintMode::MatchCount = 0;
TArray<TWeakObjectPtr<AFGBuildable>> FUSSSameSwatchPaintMode::HighlightedBuildables;
TWeakObjectPtr<AFGCharacterPlayer> FUSSSameSwatchPaintMode::HighlightCharacter;
TWeakObjectPtr<AActor> FUSSSameSwatchPaintMode::LightweightInstigator;
TWeakObjectPtr<AFGBlueprintProxy> FUSSSameSwatchPaintMode::HighlightedProxy;
TWeakObjectPtr<UClass> FUSSSameSwatchPaintMode::HighlightedSwatch;
TWeakObjectPtr<AFGBuildable> FUSSSameSwatchPaintMode::HighlightedFocus;
TWeakObjectPtr<UClass> FUSSSameSwatchPaintMode::LastReportedSwatch;
int32 FUSSSameSwatchPaintMode::LastReportedCount = 0;
bool FUSSSameSwatchPaintMode::LastReportedActive = false;
bool FUSSSameSwatchPaintMode::LastReportedLocked = false;
TSet<TWeakObjectPtr<AFGBuildable>> FUSSSameSwatchPaintMode::PreviewedBuildables;
TWeakObjectPtr<UClass> FUSSSameSwatchPaintMode::LastPreviewDescClass;
uint8 FUSSSameSwatchPaintMode::LastPreviewPatternRotation = 0;
TWeakObjectPtr<AFGBuildable> FUSSSameSwatchPaintMode::LastGameFocusedBuildable;

bool FUSSSameSwatchPaintMode::IsModeActive(AFGBuildGun* gun)
{
	return gun && gun->GetCurrentBuildGunMode() == UUSSPaintSameSwatchModeDescriptor::StaticClass();
}

UClass* FUSSSameSwatchPaintMode::GetBuildableSwatchClass(AFGBuildable* buildable)
{
	if (!IsValid(buildable))
	{
		return nullptr;
	}
	return buildable->GetCustomizationData_Native().SwatchDesc.Get();
}

bool FUSSSameSwatchPaintMode::IsLocked()
{
	return bLocked;
}

UClass* FUSSSameSwatchPaintMode::GetReferenceSwatch()
{
	return bLocked ? LockedSwatch.Get() : LiveSwatch.Get();
}

int32 FUSSSameSwatchPaintMode::GetMatchCount()
{
	return MatchCount;
}

void FUSSSameSwatchPaintMode::ToggleLock()
{
	if (bLocked)
	{
		bLocked = false;
		LockedSwatch = nullptr;
		LockedProxy = nullptr;
		LockedFocus = nullptr;
		return;
	}

	// Lock onto whatever the last highlight tick saw under the cursor. No-op if there's no swatch
	// (not aiming at a painted element), so we never lock onto an empty reference. The proxy may be
	// null (standalone element) -- that's fine, the focused element alone is the locked target.
	if (LiveSwatch.IsValid())
	{
		LockedSwatch = LiveSwatch;
		LockedProxy = LiveProxy;
		LockedFocus = LiveFocus;
		bLocked = true;
	}
}

bool FUSSSameSwatchPaintMode::ResolveReference(UFGBuildGunStatePaint* paint, AFGBuildGun* gun, AFGBlueprintProxy*& outProxy, UClass*& outSwatch, AFGBuildable*& outFocused)
{
	if (bLocked)
	{
		// Frozen: ignore the cursor entirely so the player can walk the plan and inspect.
		outProxy = LockedProxy.Get();
		outSwatch = LockedSwatch.Get();
		outFocused = LockedFocus.Get();
		return outSwatch != nullptr; // a swatch is enough (proxy is null for a standalone element)
	}

	// ResolveAimedBuildable returns the aimed building -- with its blueprint proxy when it belongs to a
	// plan, or proxy-less for a standalone element (its last-resort path).
	AFGBuildable* aimed = gun ? FUSSBuildGunPaintMode::ResolveAimedBuildable(paint, gun->GetHitResult().GetActor()) : nullptr;
	outFocused = aimed;
	outProxy = aimed ? aimed->GetBlueprintProxy() : nullptr;
	outSwatch = aimed ? GetBuildableSwatchClass(aimed) : nullptr;

	// Remember the live reference so ToggleLock() can capture it and the HUD can read it.
	LiveProxy = outProxy;
	LiveSwatch = outSwatch;
	LiveFocus = aimed;
	return outSwatch != nullptr; // need a swatch; proxy optional (standalone element with no plan)
}

void FUSSSameSwatchPaintMode::BroadcastInfo(UWorld* world, bool bActive)
{
	// Normalise the reported state (nothing meaningful to show when the mode isn't active).
	UClass* swatch = bActive ? GetReferenceSwatch() : nullptr;
	const int32 count = bActive ? MatchCount : 0;
	const bool locked = bActive && bLocked;

	if (swatch == LastReportedSwatch.Get() &&
		count == LastReportedCount &&
		bActive == LastReportedActive &&
		locked == LastReportedLocked)
	{
		return; // unchanged -> don't spam the HUD
	}

	UUniversalSwatchSlotsWorldModule* module = FUSSBuildGunPaintMode::GetWorldModule(world);
	if (!module)
	{
		return; // module not ready -> retry on a later tick (don't commit LastReported yet)
	}

	LastReportedSwatch = swatch;
	LastReportedCount = count;
	LastReportedActive = bActive;
	LastReportedLocked = locked;

	module->OnSameSwatchInfoChanged.Broadcast(swatch, count, bActive, locked);
}

void FUSSSameSwatchPaintMode::UpdateHighlight(UFGBuildGunState* paintState)
{
	AFGBuildGun* gun = paintState ? paintState->GetBuildGun() : nullptr;
	AFGCharacterPlayer* character = gun ? Cast<AFGCharacterPlayer>(gun->GetInstigator()) : nullptr;

	if (!gun || !character || !character->IsLocallyControlled() || !IsModeActive(gun))
	{
		ClearHighlight();
		MatchCount = 0;
		BroadcastInfo(paintState ? paintState->GetWorld() : nullptr, false);
		return;
	}

	UFGBuildGunStatePaint* paint = Cast<UFGBuildGunStatePaint>(paintState);
	UWorld* world = paintState->GetWorld();

	AFGBlueprintProxy* proxy = nullptr;
	UClass* refSwatch = nullptr;
	AFGBuildable* focused = nullptr;
	if (!ResolveReference(paint, gun, proxy, refSwatch, focused))
	{
		// Mode active but not aiming at a painted element (and not locked) -> nothing to show.
		ClearHighlight();
		MatchCount = 0;
		BroadcastInfo(world, true);
		return;
	}

	AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(world);

	// Rebuild the actor set + (re)acquire the lightweight instigator when the matched (proxy, swatch)
	// changes -- or, for a standalone element with no plan, when the focused element changes. While
	// locked, none of these change, so the set stays frozen.
	const bool bRebuild = proxy != HighlightedProxy.Get() || refSwatch != HighlightedSwatch.Get()
		|| (proxy == nullptr && focused != HighlightedFocus.Get());
	if (bRebuild)
	{
		ClearHighlight();

		if (proxy)
		{
			TArray<AFGBuildable*> buildables;
			proxy->CollectBuildables(buildables);
			for (AFGBuildable* buildable : buildables)
			{
				if (buildable && GetBuildableSwatchClass(buildable) == refSwatch)
				{
					HighlightedBuildables.Add(buildable);
				}
			}

			// Keep the plan's lightweight instances spawned as temporary actors while we highlight, so
			// foundations/walls can be outlined (the subsystem reclaims them otherwise).
			if (lightweightSubsystem)
			{
				if (UBoxComponent* box = proxy->GetBoundingBox())
				{
					const float radius = box->GetScaledBoxExtent().Size();
					LightweightInstigator = lightweightSubsystem->AddInstanceConverterInstigator(
						radius, nullptr, FTransform(box->GetComponentLocation()));
				}
			}
		}

		HighlightedProxy = proxy;
		HighlightedSwatch = refSwatch;
		HighlightedFocus = focused;
		HighlightCharacter = character;
	}

	// Count the matching elements (recomputed each tick -> always accurate). Track whether the focused
	// element got counted so we can add it once at the end: a focused lightweight is converted to a
	// temporary actor while aimed (its runtime data is momentarily null, so the loop skips it), and a
	// standalone element has no plan to enumerate at all.
	int32 count = 0;
	bool focusCounted = false;

	if (proxy)
	{
		TArray<AFGBuildable*> buildables;
		proxy->CollectBuildables(buildables);
		for (AFGBuildable* buildable : buildables)
		{
			if (buildable && GetBuildableSwatchClass(buildable) == refSwatch)
			{
				++count;
				if (buildable == focused)
				{
					focusCounted = true;
				}
			}
		}

		// Lightweight instances: filter by swatch, keep temps spawned for the matches (to outline them).
		if (lightweightSubsystem)
		{
			if (AActor* instigator = LightweightInstigator.Get())
			{
				if (UBoxComponent* box = proxy->GetBoundingBox())
				{
					lightweightSubsystem->SetInstanceInstigatorLocation(instigator, box->GetComponentLocation());
				}
			}

			for (const FBuildableClassLightweightIndices& entry : proxy->GetLightweightClassAndIndices())
			{
				for (int32 index : entry.Indices)
				{
					FRuntimeBuildableInstanceData* runtimeData =
						lightweightSubsystem->GetRuntimeDataForBuildableClassAndIndex(entry.BuildableClass, index);
					if (!runtimeData || runtimeData->CustomizationData.SwatchDesc.Get() != refSwatch)
					{
						continue; // doesn't share the reference swatch
					}

					++count;

					bool didSpawn = false;
					if (FInstanceToTemporaryBuildable* tempInfo =
						lightweightSubsystem->FindOrSpawnBuildableForRuntimeData(entry.BuildableClass, runtimeData, index, didSpawn))
					{
						if (tempInfo->Buildable)
						{
							HighlightedBuildables.AddUnique(tempInfo->Buildable);
							if (tempInfo->Buildable == focused)
							{
								focusCounted = true; // focused lightweight enumerated normally -> no double count
							}
						}
					}
				}
			}
		}
	}

	// Always include the focused element itself: the temp'd lightweight skipped above, or a standalone
	// element with no plan. Guarantees the count is never 0 while aiming at a painted element.
	if (IsValid(focused) && !focusCounted && GetBuildableSwatchClass(focused) == refSwatch)
	{
		++count;
		HighlightedBuildables.AddUnique(focused);
	}

	MatchCount = count;

	// --- Live colour preview on the matching elements (visual only), mirroring the Blueprint mode ---
	const TSubclassOf<UFGCustomizationRecipe> activeRecipe = paint ? paint->GetActiveRecipe() : nullptr;
	UClass* activeDescClass = activeRecipe ? *UFGCustomizationRecipe::GetCustomizationDescriptor(activeRecipe) : nullptr;
	AFGGameState* gameState = world ? world->GetGameState<AFGGameState>() : nullptr;
	UUniversalSwatchSlotsWorldModule* module = FUSSBuildGunPaintMode::GetWorldModule(world);

	// Writing per-instance colour data on instanced meshes races with lightweight replication on a
	// client (BeginWriteAccess assert). So only preview colour when we are the authority, unless the
	// player opts in via the world-module toggle. The outline below is applied in all cases.
	const bool bCanPreviewColor = (world && world->GetNetMode() != NM_Client) || (module && module->GetClientPreview());

	// Re-apply the preview once per building, and again when the active customization changes.
	const uint8 currentPatternRotation = paint ? paint->mPatternRotation : 0;
	if (LastPreviewDescClass.Get() != activeDescClass || LastPreviewPatternRotation != currentPatternRotation)
	{
		PreviewedBuildables.Reset();
		LastPreviewDescClass = activeDescClass;
		LastPreviewPatternRotation = currentPatternRotation;
	}

	// The game previews on the building you aim at and reverts it when you move off, wiping our preview
	// on that one -- evict it so the loop below re-applies it once.
	AFGBuildable* gameFocus = paint ? Cast<AFGBuildable>(paint->mCurrentlyAimedAtActor) : nullptr;
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

		// Outline (safe to refresh each tick; the game clears it when the cursor leaves a building).
		IFGColorInterface::Execute_StartIsAimedAtForColor(buildable, character, true);

		// Colour preview once per building with the current customization (visual only -> revertible).
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
				previewData.PatternRotation = paint->AlignPatternRotationWithActor(buildable, paint->mPatternRotation);
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

	// Notify the HUD (only fires when the swatch / count / lock / active state actually changes).
	BroadcastInfo(world, true);
}

void FUSSSameSwatchPaintMode::ClearHighlight()
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
		// Revert the colour preview we applied (restore the building's real look).
		if (PreviewedBuildables.Contains(weakBuildable))
		{
			const FFactoryCustomizationData& realData = buildable->GetCustomizationData_Native();
			if (realData.IsInitialized())
			{
				buildable->ApplyCustomizationData_Native(realData);
			}
		}
	}
	HighlightedBuildables.Reset();
	PreviewedBuildables.Reset();
	LastPreviewDescClass = nullptr;
	LastPreviewPatternRotation = 0;
	LastGameFocusedBuildable = nullptr;

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

	LightweightInstigator = nullptr;
	HighlightedProxy = nullptr;
	HighlightedSwatch = nullptr;
	HighlightedFocus = nullptr;
	HighlightCharacter = nullptr;
}

void FUSSSameSwatchPaintMode::OnEndPaintState(UFGBuildGunState* paintState)
{
	ClearHighlight();
	bLocked = false;
	LockedSwatch = nullptr;
	LockedProxy = nullptr;
	LockedFocus = nullptr;
	LiveSwatch = nullptr;
	LiveProxy = nullptr;
	LiveFocus = nullptr;
	MatchCount = 0;
	BroadcastInfo(paintState ? paintState->GetWorld() : nullptr, false);
}

void FUSSSameSwatchPaintMode::ResetAfterPaint()
{
	// Drop the highlight + colour preview and release the lock: the matched elements now carry the new
	// customization, so leaving the old selection highlighted would be stale. The next aim re-evaluates
	// live (the very next TickState re-broadcasts the HUD info). Runs client-side (see RegisterHooks).
	ClearHighlight();
	bLocked = false;
	LockedSwatch = nullptr;
	LockedProxy = nullptr;
	LockedFocus = nullptr;
}

void FUSSSameSwatchPaintMode::ApplyToPlanFiltered(UFGBuildGunStatePaint* self, FFactoryCustomizationData* customizationData, AFGBlueprintProxy* proxy, UClass* referenceSwatch)
{
	if (!self || !proxy || !customizationData || !referenceSwatch)
	{
		return;
	}

	// Actor buildables (machines, etc.) whose current swatch matches the reference.
	TArray<AFGBuildable*> buildables;
	proxy->CollectBuildables(buildables);
	for (AFGBuildable* buildable : buildables)
	{
		if (buildable && GetBuildableSwatchClass(buildable) == referenceSwatch)
		{
			buildable->SetCustomizationData_Native(*customizationData); // skipCombine=false -> merges with existing
			buildable->FlushNetDormancy(); // replicate live to connected clients
		}
	}

	// Lightweight instances (foundations, walls, ...) whose runtime data shares the reference swatch.
	if (AFGLightweightBuildableSubsystem* lightweightSubsystem = AFGLightweightBuildableSubsystem::Get(self->GetWorld()))
	{
		for (const FBuildableClassLightweightIndices& entry : proxy->GetLightweightClassAndIndices())
		{
			for (int32 index : entry.Indices)
			{
				FRuntimeBuildableInstanceData* runtimeData =
					lightweightSubsystem->GetRuntimeDataForBuildableClassAndIndex(entry.BuildableClass, index);
				if (!runtimeData || runtimeData->CustomizationData.SwatchDesc.Get() != referenceSwatch)
				{
					continue; // doesn't share the reference swatch
				}

				bool didSpawn = false;
				FInstanceToTemporaryBuildable* tempInfo =
					lightweightSubsystem->FindOrSpawnBuildableForRuntimeData(entry.BuildableClass, runtimeData, index, didSpawn);

				if (tempInfo && tempInfo->Buildable)
				{
					tempInfo->Buildable->SetCustomizationData_Native(*customizationData); // merges
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

void FUSSSameSwatchPaintMode::RegisterHooks()
{
	// Locked-fire guard + post-paint cleanup, hooked at the CLIENT-side fire entry. We must gate here,
	// NOT in the paint state's PrimaryFire_Implementation: the engine notes "all the actions are executed
	// on the server but may be simulated on the client" -- in multiplayer the client's OnPrimaryFirePressed
	// sends Server_PrimaryFire and the SERVER runs PrimaryFire_Implementation, where our client-side lock
	// state doesn't exist. So a guard there never sees the lock (works in single player, fails on a client).
	// OnPrimaryFirePressed runs on the local player BEFORE the server RPC, so cancelling it stops the fire
	// on every machine.
	SUBSCRIBE_METHOD(AFGBuildGun::OnPrimaryFirePressed,
		[](auto& scope, AFGBuildGun* gun)
		{
			if (!gun || !IsModeActive(gun))
			{
				return; // not our mode -> auto-forward the normal fire
			}

			if (bLocked)
			{
				// While locked, only allow firing at an element of the locked swatch, so an accidental
				// click off the set never paints a different group.
				UFGBuildGunStatePaint* paint = Cast<UFGBuildGunStatePaint>(gun->GetCurrentState());
				AFGBuildable* aimed = paint ? FUSSBuildGunPaintMode::ResolveAimedBuildable(paint, gun->GetHitResult().GetActor()) : nullptr;
				UClass* aimedSwatch = aimed ? GetBuildableSwatchClass(aimed) : nullptr;
				if (aimedSwatch != GetReferenceSwatch())
				{
					scope.Cancel(); // aiming off the locked set -> ignore the click (no fire, no server RPC)
					return;
				}
			}

			scope(gun); // initiate the fire (client -> server)

			// Paint initiated -> drop the highlight + release the lock so it doesn't linger on the elements
			// just repainted. Runs client-side here, so it works in multiplayer too (unlike the old hook).
			ResetAfterPaint();
		});
	UE_LOG(LogUSS, Verbose, TEXT("Registered Same-Swatch primary-fire lock guard."));
}
