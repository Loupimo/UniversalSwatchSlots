#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UFGBuildGunState;
class UFGBuildGunStatePaint;
class AFGBuildGun;
class AFGBuildable;
class AFGBlueprintProxy;
class AFGCharacterPlayer;
struct FFactoryCustomizationData;

/**
 * "Same Swatch" paint mode.
 *
 * A variant of the Blueprint paint mode: instead of painting the whole plan, it only affects the
 * plan elements that share the SAME swatch as the element you aim at (the "reference" swatch). It
 * highlights only those elements and reports the reference swatch descriptor + the matching count
 * so the HUD can display them.
 *
 * A client-side "lock" (bind a key, e.g. H, to ToggleLock() in Blueprint) freezes the reference and
 * the highlight so the player can walk around the plan and inspect exactly what will be affected,
 * without the highlight changing as the cursor moves. While locked, painting only applies if the
 * aimed element shares the locked swatch (so an accidental click off the set does nothing).
 *
 * Architecture note: everything here is CLIENT-side (highlight, lock, reference, count) except the
 * actual paint, which runs server-side in Server_ExecutePaint. The server derives the reference
 * swatch from the aimed element (hitActor), exactly like the Blueprint mode resolves the plan -- so
 * the feature is robust on dedicated servers without trying to ship client lock state to the server.
 */
class FUSSSameSwatchPaintMode
{
public:
	/** Installs this mode's own hooks (the locked-fire guard). Call once from module startup,
	 *  alongside FUSSBuildGunPaintMode::RegisterHooks(). */
	static void RegisterHooks();

	/** True when our "Same Swatch" build mode is the build gun's current mode. */
	static bool IsModeActive(AFGBuildGun* gun);

	/** Per-tick highlight: outlines only the plan elements sharing the reference swatch and refreshes
	 *  the reference + match count. Clears its highlight when the mode isn't active. Driven from the
	 *  paint state's TickState hook. */
	static void UpdateHighlight(UFGBuildGunState* paintState);

	/** Clears the highlight and resets the lock, and broadcasts an "inactive" HUD update. Call when
	 *  leaving the Paint state. */
	static void OnEndPaintState(UFGBuildGunState* paintState);

	/** Toggle the inspection lock (client-side). Locks onto the swatch currently aimed at so the
	 *  player can walk the plan; toggling again (or aiming nothing when unlocked) releases it.
	 *  No-op if there is no valid swatch under the cursor when locking. */
	static void ToggleLock();

	/** True while the inspection lock is engaged. */
	static bool IsLocked();

	/** The reference swatch descriptor currently in effect (locked one if locked, else the aimed
	 *  one). Null when not aiming at a painted plan element. Returned as UClass* for Blueprint. */
	static UClass* GetReferenceSwatch();

	/** Number of plan elements (actors + lightweight instances) sharing the reference swatch. */
	static int32 GetMatchCount();

	/** The swatch descriptor class a buildable currently uses (its customization SwatchDesc), or
	 *  null. Public so the server paint path can resolve the reference from the aimed element. */
	static UClass* GetBuildableSwatchClass(AFGBuildable* buildable);

	/** Server-side: apply customizationData to every plan element whose current swatch equals
	 *  referenceSwatch (actor buildables + lightweight instances). */
	static void ApplyToPlanFiltered(UFGBuildGunStatePaint* self, FFactoryCustomizationData* customizationData, AFGBlueprintProxy* proxy, UClass* referenceSwatch);

private:
	/** Removes the outline + colour preview previously applied and releases the lightweight instigator. */
	static void ClearHighlight();

	/** After a paint: drop the highlight and release the lock so it doesn't linger on the elements that
	 *  were just repainted; the next aim starts a fresh selection. Client-side. */
	static void ResetAfterPaint();

	/** Resolves the active reference (lock-aware): proxy (may be null for a standalone element not in a
	 *  plan), swatch, and the focused element. Updates the live reference when unlocked. Returns false
	 *  only when there is no swatch to act on. */
	static bool ResolveReference(UFGBuildGunStatePaint* paint, AFGBuildGun* gun, AFGBlueprintProxy*& outProxy, UClass*& outSwatch, AFGBuildable*& outFocused);

	/** Broadcasts the world module's OnSameSwatchInfoChanged event, but only when the reported values
	 *  (swatch, count, active, locked) actually change -- so it never spams the HUD every tick. */
	static void BroadcastInfo(UWorld* world, bool bActive);

	//~ Last values reported to the HUD event (change-detection for BroadcastInfo) ----
	static TWeakObjectPtr<UClass> LastReportedSwatch;
	static int32 LastReportedCount;
	static bool LastReportedActive;
	static bool LastReportedLocked;

	//~ Lock / reference state (client-side) -----------------------------------------
	static bool bLocked;
	static TWeakObjectPtr<UClass> LockedSwatch;
	static TWeakObjectPtr<AFGBlueprintProxy> LockedProxy;
	static TWeakObjectPtr<AFGBuildable> LockedFocus;
	static TWeakObjectPtr<UClass> LiveSwatch;   // last reference under the cursor (for lock capture + UI)
	static TWeakObjectPtr<AFGBlueprintProxy> LiveProxy;
	static TWeakObjectPtr<AFGBuildable> LiveFocus;
	static int32 MatchCount;

	//~ Highlight state (own set, independent from FUSSBuildGunPaintMode) -------------
	static TArray<TWeakObjectPtr<AFGBuildable>> HighlightedBuildables;
	static TWeakObjectPtr<AFGCharacterPlayer> HighlightCharacter;
	static TWeakObjectPtr<AActor> LightweightInstigator;
	static TWeakObjectPtr<AFGBlueprintProxy> HighlightedProxy;  // proxy the current set was built for
	static TWeakObjectPtr<UClass> HighlightedSwatch;            // swatch the current set was built for
	static TWeakObjectPtr<AFGBuildable> HighlightedFocus;      // focused element the set was built for (standalone case)

	//~ Colour-preview state (visual only, applied once per building) -----------------
	static TSet<TWeakObjectPtr<AFGBuildable>> PreviewedBuildables;
	static TWeakObjectPtr<UClass> LastPreviewDescClass;
	static uint8 LastPreviewPatternRotation;
	static TWeakObjectPtr<AFGBuildable> LastGameFocusedBuildable;
};
