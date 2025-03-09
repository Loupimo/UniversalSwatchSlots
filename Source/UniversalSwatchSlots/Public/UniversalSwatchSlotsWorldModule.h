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
	 * @param	Subsystem			The USS subsytem to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void InitUSSGameWorldModule(AUniversalSwatchSlotsSubsystem* Subsystem);

public:

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMoreSwatchSlots;

	/* The subsystem used to load and store swatch data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	AUniversalSwatchSlotsSubsystem* USSSubsystem = nullptr;
};