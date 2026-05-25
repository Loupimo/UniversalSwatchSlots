// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"

#include "UniversalSwatchSlotsSubsystem.h"

#include "UniversalSwatchSlotsWorldModule.generated.h"


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
	 * 
	 * Note: The adding should be done inside blueprint. Otherwise cause crash for unknown reason...
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	TArray<FGlobalColorPreset> ApplySwatchesColorOptionToPreset(TArray<UUSSSwatchDesc*> SwatchDescriptions, bool AddPrimaryColors, bool AddSecondaryColors, bool RemovePrimayColors, bool RemoveSecondaryColors);

public:

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMoreSwatchSlots;

	/* The subsystem used to load and store swatch data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	AUniversalSwatchSlotsSubsystem* USSSubsystem = nullptr;
};