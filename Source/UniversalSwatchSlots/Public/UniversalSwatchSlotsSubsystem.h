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
	void UpdateSavedSwatches(TArray<UUSSSwatchDesc*> ToSave);


	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void UpdateSwatchesArray(TMap<int32, UUSSSwatchGroup*> Groups, TMap<int32, UUSSSwatchDesc*> Descs, TMap<int32, UUSSSwatchRecipe*> Recipes);

public:

	/* A detailed array that contains info about saved swatches. */
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FUSSSwatchSaveInfo> SavedSwatches;

	/* The list of all available swatch groups. This array is modified when the GenerateDynamicSwatchGroup function is called. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TArray<UUSSSwatchGroup*> SwatchGroupArray;

	/* The list of all generated swatch descriptors. This array is modified when the GenerateDynamicSwatchDescriptor function is called.  */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TArray<UUSSSwatchDesc*> SwatchDescriptorArray;

	/* The build gun blueprint class that will be used by the dynamically created swatch recipes. This array is modified when the GenerateDynamicSwatchRecipe function is called. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TArray<UUSSSwatchRecipe*> SwatchRecipeArray;


	/* The list of all available swatch groups. This array is modified when the GenerateDynamicSwatchGroup function is called. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	FUSSPalette SwatchPalette;
};
