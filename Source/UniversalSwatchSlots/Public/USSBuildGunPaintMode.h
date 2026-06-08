#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"


class UUSSPaintModeWidget;
class UFGBuildGunState;
class AFGBuildable;
class AFGBlueprintProxy;
class AFGCharacterPlayer;
class UUniversalSwatchSlotsWorldModule;
class UWorld;

/**
 * Adds a "Default / Blueprint" build-mode toggle to the build gun's Paint state,
 * and shows a HUD indicator widget while the player is painting.
 *
 * Everything is wired through SML hooks installed by RegisterHooks(), which must be
 * called once at module startup (runtime only).
 *
 * The indicator widget class is NOT hard-coded: it is read at runtime from
 * UUniversalSwatchSlotsWorldModule::PaintModeWidgetClass, which is set in the
 * RootGameWorld_UniversalSwatchSlots blueprint.
 */
class FUSSBuildGunPaintMode
{
public:
	/** Installs the build-gun hooks. Call once from module startup. */
	static void RegisterHooks();

private:
	/** Creates and shows the paint-mode indicator for the local player. */
	static void ShowIndicator(UFGBuildGunState* paintState);

	/** Removes the paint-mode indicator if present. */
	static void HideIndicator();

	/** Outlines every buildable of the aimed blueprint while in Blueprint paint mode. */
	static void UpdateBlueprintHighlight(UFGBuildGunState* paintState);

	/** Removes the blueprint outline previously applied, if any. */
	static void ClearBlueprintHighlight();

	/** Resolves this mod's world module from the given world (nullptr if unavailable). */
	static UUniversalSwatchSlotsWorldModule* GetWorldModule(UWorld* world);

	/** Current indicator instance (local player only). Kept alive by the viewport. */
	static TWeakObjectPtr<UUSSPaintModeWidget> IndicatorWidget;

	/** Blueprint currently outlined for preview (local player only). */
	static TWeakObjectPtr<AFGBlueprintProxy> HighlightedProxy;
	static TArray<TWeakObjectPtr<AFGBuildable>> HighlightedBuildables;
	static TWeakObjectPtr<AFGCharacterPlayer> HighlightCharacter;

	/** Instance-converter instigator keeping the plan's lightweight instances spawned as temps. */
	static TWeakObjectPtr<AActor> LightweightInstigator;
};
