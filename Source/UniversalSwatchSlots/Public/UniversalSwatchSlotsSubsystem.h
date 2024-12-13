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
	
	//AUniversalSwatchSlotsSubsystem();

public:

	UFUNCTION(BlueprintPure, Category = "Subsystem", DisplayName = "GetAUniversalSwatchSlotsSubsystem", meta = (DefaultToSelf = "WorldContext"))
	static AUniversalSwatchSlotsSubsystem* Get(UObject* worldContext);


	/**
	 * Add new swatch color slots to the gamestate using the given swatch descriptors.
	 *
	 * @param	SwatchDescriptions		The swatch descriptors to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void AddNewSwatchesColorSlotsToGameState(TArray<UUSSSwatchDesc* > SwatchDescriptions);

	/**
	 * Create a new swatch using the desired group ID and swatch name.
	 *
	 * Note: This function will create a new swatch group if the given group ID doensn't exist in the SwatchGroupArray and call GenerateDynamicSwatchGroup -> GenerateDynamicSwatchDescriptor -> GenerateDynamicSwatchRecipe functions. This function does nothing if the swatch descriptor / recipe already exist.
	 *
	 * @param	SwatchInformation			The swatch information to use.
	 * @param	SwatchGroup					The used swatch group. NULL if the function was aborted.
	 * @param	SwatchDescriptor			The generated swatch descriptor. NULL if the function was aborted.
	 * @param	SwatchRecipe				The generated swatch recipe. NULL if the function was aborted.
	 *
	 * @return True if the swatch was created, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool GenerateNewSwatchUsingInfo(FUSSSwatch SwatchInformation, UUSSSwatchGroup*& SwatchGroup, UUSSSwatchDesc*& SwatchDescriptor, UUSSSwatchRecipe*& SwatchRecipe);

	/**
	 * Create a new swatch group using the desired ID, name and priority.
	 *
	 * Note : The newly generated swatch group will be added to the SwatchGroupArray at the given UniqueGroupID.
	 *
	 * @param	UniqueGroupID			The ID to give to the swatch group.
	 * @param	DisplayName				The name to give to the swatch group.
	 * @param	Priority				The priority to give to the swatch group.
	 *
	 * @warning Be sure that the desired swatch group doesn't exist otherwise its CDO will be modified.
	 *
	 * @return The newly generated swatch descriptor, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UUSSSwatchGroup* GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority);

	/**
	 * Create a new swatch descriptor using the desired swatch group, name and ID.
	 *
	 * @param	SlotID					The slot ID used by swatch descriptor.
	 * @param	SwapID					The ID to give to the potential present swatch at slot ID. Be sure that Swap slot ID is available as it will be overwritten.
	 * @param	DisplayName				The name to give to the swatch descriptor.
	 * @param	DisplayName				The generated name to give to the swatch descriptor class.
	 * @param	Priority				The priority to give to this swatch.
	 * @param   PrimaryColor			The primary color used to generate the swatch icon.
	 * @param	SecondaryColor			The secondary color used to generate the swatch icon.
	 * @param	SwatchGroup				The swatch group to use.
	 * @param	HasSwapped				Tells if a swap has occured.
	 *
	 * @warning Be sure that the desired swatch descriptor doesn't exist otherwise its CDO will be modified.
	 *
	 * @return The newly generated swatch descriptor, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UUSSSwatchDesc* GenerateDynamicSwatchDescriptor(int32 SlotID, FText DisplayName, FString GenName, float Priority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory* SwatchGroup);

	/**
	 * Create a new swatch recipe using the desired swatch descriptor.
	 *
	 * @param	SwatchDescriptor		The swatch descriptor to use.
	 *
	 * @warning Be sure that a recipe for the given swatch descriptor doesn't exist otherwise its CDO will be modified.
	 *
	 * @return The newly generated swatch recipe, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UUSSSwatchRecipe* GenerateDynamicSwatchRecipe(int32 UniqueID, UUSSSwatchDesc* SwatchDescriptor);


	/**
	 * Patch existing buildings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Subsystem")
	void PatchBuildingsSwatchDescriptor();


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

	/**
	 * Update the USS version used for saving this file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Subsystem")
	void UpdateSavedVersion();

	void RetrieveFreeColorSlotID();

public:

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMSS;

	/* The build gun blueprint class that will be used by the dynamically created swatch recipes. */
	UPROPERTY(EditDefaultsOnly, Category = "Swatch")
	TSoftClassPtr<UObject> BuildGunBPClass;

	/* The category under which the generated swatch group should appear. */
	UPROPERTY(EditDefaultsOnly, Category = "Swatch")
	TSubclassOf<UFGCustomizerCategory> SwatchCategory = UFGCustomizerCategory::StaticClass();

	/* The paint finish to apply to our swatches. */
	UPROPERTY(EditDefaultsOnly, Category = "Swatch")
	TSubclassOf<UFGFactoryCustomizationDescriptor_PaintFinish>PaintFinish;

	/* The description that should be added to all swatches. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	FText SwatchDescription;

	/* The list of all available swatch groups. This array is modified when the GenerateDynamicSwatchGroup function is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, UUSSSwatchGroup*> SwatchGroupArray;

	/* The list of all generated swatch descriptors. This array is modified when the GenerateDynamicSwatchDescriptor function is called.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, UUSSSwatchDesc*> SwatchDescriptorArray;

	/* The build gun blueprint class that will be used by the dynamically created swatch recipes. This array is modified when the GenerateDynamicSwatchRecipe function is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, UUSSSwatchRecipe*> SwatchRecipeArray;

	/* A detailed array that contains info about saved swatches. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Saving")
	TArray<FUSSSwatchSaveInfo> SavedSwatches;

	/* Gives the USS version used by the save file. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Saving")
	EUSSVersion SaveVersion = EUSSVersion::None;

	/* Tells is More swatch slots was installed when the game was saved. */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Saving")
	bool WasUsingMSS = false;


protected:

	/* This map contains the number of time a unique swatch name appears in the configuration. */
	UPROPERTY()
	TMap<FString, int32> SwatchNameCount;

	/* This array contains all the free color slot IDs. */
	UPROPERTY()
	TArray<int32> ValidSlotIDs;

	/* The internal map to check if all previous saved swatches have a macthing loaded swatch. It the key is not equal to the value we should get all actor of FGBuilding type to re-reference the swatches. */
	TMap <int, int> InternalSwatchMatch;
};
