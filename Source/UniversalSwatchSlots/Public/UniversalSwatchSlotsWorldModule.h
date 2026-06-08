// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "ItemAmount.h"

#include "UniversalSwatchSlotsSubsystem.h"

#include "UniversalSwatchSlotsWorldModule.generated.h"

class UUSSPaintModeWidget;
class AFGBuildGun;


/**
 *
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUniversalSwatchSlotsWorldModule : public UGameWorldModule
{
	GENERATED_BODY()

	/**
	 * Generate the groups, swatches descriptors and receipe of the given palette.
	 *
	 * @param	Palette				The palette to generate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatches")
	void GenerateSwatchesFromPalette(FUSSPalette Palette);

	/**
	 * Initialize the USS game world module using the given USS subsystem.
	 *
	 * @param	USSInstance			The USS instance to use.
	 * @param	Subsystem			The USS subsytem to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void InitUSSGameWorldModule(UUniversalSwatchSlotsGIModule* USSInstance, AUniversalSwatchSlotsSubsystem* Subsystem);

	/**
	 * Remove all colors from player preset and create the array that should be added instead.
	 *
	 * @param	SwatchDescriptions		The swatch descriptors to use.
	 * @param	AddPrimaryColors		Tell if we should add the primary colors to the final array.
	 * @param	AddSecondaryColors		Tell if we should add the secondary colors to the final array.
	 * @param	RemovePrimayColors		Tell if we should remove the primary colors from the final array.
	 * @param	RemoveSecondaryColors	Tell if we should remove the primary colors from the final array.
	 * 
	 * @return A sorted array containing all the player preset color to add.
	 * 
	 * @Warning: The adding should be done inside blueprint otherwise player will not be able to join the host.
	 *
	 * @Note: If AddXColors and RemoveXColors are true the Add operation will be executed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	TArray<FGlobalColorPreset> ApplySwatchesColorOptionToPreset(TArray<UUSSSwatchDesc*> SwatchDescriptions, bool AddPrimaryColors, bool AddSecondaryColors, bool RemovePrimayColors, bool RemoveSecondaryColors);

public:

	/* Tells if client should render the preview customization in blueprint mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsClientPreviewEnabled;

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMoreSwatchSlots;

	/* The subsystem used to load and store swatch data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<AUniversalSwatchSlotsSubsystem> USSSubsystem = nullptr;

	/* HUD widget shown while painting, to display/cycle the paint build mode (Default / Blueprint).
	   Must be a Widget Blueprint reparented to UUSSPaintModeWidget.
	   Set this in the RootGameWorld_UniversalSwatchSlots blueprint defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUSSPaintModeWidget> PaintModeWidgetClass;

	/**
	 * Total resource cost to paint the blueprint the build gun is currently aiming at, with the
	 * active customization, in Blueprint paint mode. This is the per-building customization cost
	 * times the number of buildings in the plan (actors + lightweights). Use it to show the
	 * required resources in the HUD before the player paints.
	 *
	 * @param	BuildGun			The local player's build gun.
	 * @param	OutCost				The total cost for the whole plan.
	 * @param	OutBuildingCount	The number of buildings the cost covers.
	 * 
	 * @return An empty cost (and 0 buildings) when not aiming at a plan, not in Paint state, or
	 * when no-build-cost is enabled.
	 */
	UFUNCTION(BlueprintPure, Category = "Paint")
	void GetBlueprintPaintCost(AFGBuildGun* BuildGun, TArray<FItemAmount>& OutCost, int32& OutBuildingCount) const;
	
	/**
	 * Get the client preview rendering state.
	 *
	 * @return True if the client should render the preview, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Config")
	bool GetClientPreview() const;
};