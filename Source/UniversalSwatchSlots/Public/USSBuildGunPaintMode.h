#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "ItemAmount.h"


class UUSSPaintModeWidget;
class UFGBuildGunState;
class AFGBuildable;
class AFGBlueprintProxy;
class AFGCharacterPlayer;
class UUniversalSwatchSlotsWorldModule;
class UWorld;
class UFGBuildGunStatePaint;
class UFGFactoryCustomizationDescriptor_Material;
struct FFactoryCustomizationData;

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

	/** Resolves the real aimed buildable for the current paint. For swatch/pattern/skin the hit
	 *  actor IS the building and carries the blueprint proxy. For materials the engine aims a
	 *  transient mesh-swap preview actor (no proxy), so we fall back to the paint state's private
	 *  aimed-at / customization-target actor (exposed via an Access Transformer friend). Public so
	 *  the HUD cost query can resolve the same building the paint will act on. */
	static AFGBuildable* ResolveAimedBuildable(UFGBuildGunStatePaint* self, AActor* hitActor);

	/** Number of buildings in a blueprint plan (actor buildables + lightweight instances). */
	static int32 CountPlanBuildings(AFGBlueprintProxy* proxy);

	/** Per-application customization cost scaled by a building count (whole-plan paint cost). */
	static TArray<FItemAmount> ScaleCost(const TArray<FItemAmount>& perApplicationCost, int32 buildingCount);

private:
	/** Applies the active paint to every other building of the blueprint (actor buildables +
	 *  lightweight instances). The proxy is captured before the original paint runs because a
	 *  material apply can reconstruct the focused building and drop its proxy link.
	 *  Used for swatch / pattern / skin (pure per-instance customization data). */
	static void ApplyToBlueprintPlan(UFGBuildGunStatePaint* self, FFactoryCustomizationData* customizationData, AFGBlueprintProxy* proxy, AFGBuildable* skipBuildable);

	/** Materials are not per-instance data: for foundations/walls a material is a different
	 *  recipe/mesh, so each plan lightweight must be removed and re-added as the swapped class,
	 *  then re-registered to the blueprint proxy (the game's own apply leaves the focused
	 *  building unlinked). Actor buildables have no material variant and are left untouched. */
	static void ApplyMaterialSwapToPlan(UFGBuildGunStatePaint* self, AFGBlueprintProxy* proxy, TSubclassOf<UFGFactoryCustomizationDescriptor_Material> material);
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

	/** Plan buildings that currently have the colour preview applied. Writing per-instance colour
	 *  data on instanced meshes is racy, so we apply the preview ONCE per building (tracked here)
	 *  instead of every tick, and re-apply only when the active customization changes. */
	static TSet<TWeakObjectPtr<AFGBuildable>> PreviewedBuildables;

	/** The customization the colour preview was last applied with; a change forces a single
	 *  re-apply across every plan building. */
	static TWeakObjectPtr<UClass> LastPreviewDescClass;
	static uint8 LastPreviewPatternRotation;

	/** The building the game is currently previewing (its aimed-at building). When this changes the
	 *  game reverts the one you just left, wiping our colour preview on it -- so we evict that
	 *  building to re-apply the preview to it once. */
	static TWeakObjectPtr<AFGBuildable> LastGameFocusedBuildable;
};
