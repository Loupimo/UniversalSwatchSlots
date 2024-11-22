// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"

#include "UniversalSwatchSlotsDefinitions.h"

#include "UniversalSwatchSlotsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API AUniversalSwatchSlotsSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "Subsystem", DisplayName = "GetAUniversalSwatchSlotsSubsystem", meta = (DefaultToSelf = "WorldContext"))
	static AUniversalSwatchSlotsSubsystem* Get(UObject* worldContext);

	/**
	 * Find the matching saved swatch if any.
	 *
	 *
	 * @param	SwatchDisplayName		The swatch display name.
	 * @param	SwatchClassAcr			The swatch class acronym.
	 * @param	SwatchID				The swatch ID. It should match the occurrence number of the swatch display name you are trying to find (starting at index 0). 
	 *
	 * @return The matching generated name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	static FString BuildSwatchGenName(FString SwatchDisplayName, FString SwatchClassAcr, int32 SwatchID);

	/**
	 * Find the matching saved swatch if any.
	 *
	 *
	 * @param	GeneratedName			The swatch generated name (use the BuildSwatchGenName function to create one).
	 * @param	Out						The saved swatch info if found, NULL otherwise.
	 *
	 * @return True if the swatch was found, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool FindSavedSwatch(FString GeneratedName, FUSSSwatchSaveInfo& Out);

	/**
	 * Update the saved swatches array.
	 *
	 *
	 * @param	GeneratedName			The swatch generated name (use the BuildSwatchGenName function to create one).
	 * @param	Out						The saved swatch info if found, NULL otherwise.
	 *
	 * @return True if the swatch was found, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void UpdateSavedSwatches(TArray<FUSSSwatchInformation> ToSave);

public:

	/* A detailed array that contains info about saved swatches. */
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FUSSSwatchSaveInfo> SavedSwatches;
};
