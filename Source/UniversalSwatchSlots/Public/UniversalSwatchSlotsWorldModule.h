// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

#include "Module/GameWorldModule.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "FGBuildGun.h"

#include "ModConfiguration.h"
#include "ConfigPropertyArray.h"

#include "UniversalSwatchSlotsSubsystem.h"
#include "UniversalSwatchSlotsDefinitions.h"
#include "UniversalSwatchSlotsGIModule.h"

#include "UniversalSwatchSlotsWorldModule.generated.h"


/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUniversalSwatchSlotsWorldModule : public UGameWorldModule
{
	GENERATED_BODY()


	/**
	 * Add new swatch color slots to the gamestate and buildable sub system using the desired swatch informations.
	 *
	 * @param	SwatchInformations		The swatch informations to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void AddNewSwatchesColorSlots(TArray<UUSSSwatchDesc* > SwatchDescriptions);

	/**
	 * Move the source class and its CDO to the target class and its CDO. Apply NewID to the target CDO and class.
	 *
	 * @param	Target			The destination class.
	 * @param	Source			The source class to move.
	 * @param	NewID			The ID to give.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void MoveCDO(UUSSSwatchDesc* Target, UUSSSwatchDesc* Source, int32 NewID);

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
	UUSSSwatchDesc* GenerateDynamicSwatchDescriptor(int32 SlotID, int32 SwapID, FText DisplayName, FString GenName, float Priority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory* SwatchGroup, bool& HasSwapped);

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
	 * Create a new swatch using the desired group ID and swatch name.
	 *
	 * Note: This function will create a new swatch group if the given group ID doensn't exist in the SwatchGroupArray and call GenerateDynamicSwatchGroup -> GenerateDynamicSwatchDescriptor -> GenerateDynamicSwatchRecipe functions. This function does nothing if the swatch descriptor / recipe already exist.
	 *
	 * @param	UniqueGroupID				The swatch group ID to use. If the group doesn't exist it will be created.
	 * @param	GroupDisplayName			The name to give to the swatch group. If the group already exist its name will be changed.
	 * @param	GroupPriority				The priority to give to the swatch group. If the group already exist its priority will be changed.
	 * @param	SwatchPriority				The priority to give to this swatch.
	 * @param	SwatchUniqueID				The swatch descriptor ID to use. If the descriptor ID is already in use this function will exit without doing anything.
	 * @param	SwatchDisplayName			The name to give to the swatch descriptor.
	 * @param   PrimaryColor				The primary color of the swatch.
	 * @param	SecondaryColor				The secondary color of the swatch.
	 * @param	SwatchGroup					The used swatch group. NULL if the function was aborted.
	 * @param	SwatchDescriptor			The generated swatch descriptor. NULL if the function was aborted.
	 * @param	SwatchRecipe				The generated swatch recipe. NULL if the function was aborted.
	 * 
	 * @return True if the swatch was created, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool GenerateNewSwatch(int32 UniqueGroupID, FText GroupDisplayName, float GroupPriority, int32 SwatchUniqueID, FText SwatchDisplayName, float SwatchPriority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UUSSSwatchGroup*& SwatchGroup, UUSSSwatchDesc*& SwatchDescriptor, UUSSSwatchRecipe*& SwatchRecipe);

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
	 * Parse the mod's configuration referenced by the variable ModConfig.
	 * 
	 * @return True if the has been correctly parsed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	bool ParseModConfig();

	/**
	 * Initialize the USS game world module.
	 *
	 * @param	CleanSlotInit			Tells if we should clean color slot or not.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void InitUSSGameWorldModule(bool CleanSlotInit = true);

public:

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
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

	/* The colection where the swatch should be added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SessionName;

	/* The configuration that contains all the swatch to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UConfigProperty* RootConfig;

	/* The collection in which all the newly added swatch descriptors will be added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UFGFactoryCustomizationCollection* SwatchCollection;

	/* The available palettes parsed from the configuration file. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TMap<FString, FUSSPalette> Palettes;

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMoreSwatchSlots;

	/* The subsystem used to load and store swatch data. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Config")
	AUniversalSwatchSlotsSubsystem* USSSubsystem = nullptr;

	/* The game instance used to construct dynamic swatch descriptor and recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UUniversalSwatchSlotsGIModule* GameInstance;

protected:

	/* This map contains the number of time a unique swatch name appears in the configuration. */
	UPROPERTY()
	TMap<FString, int32> SwatchNameCount;

	/* This array contains all the free color slot IDs. */
	UPROPERTY()
	TArray<int32> ValidSlotIDs;

	/* The list of all generated class. */
	UPROPERTY()
	TArray<UClass*> GeneratedClasses;
	
	// The value of the custom color if any
	UPROPERTY()
	FFactoryCustomizationColorSlot CustomColor;


	UClass* GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName);

	
	void ParseAssociations(UConfigPropertyArray* Associations);
	void ParsePalettes(UConfigPropertyArray* PalettesArr);
	int32 ParseSwatchGroup(int32 StartValidSlotID, int32 GroupID, UConfigPropertySection* SwatchGroup, FUSSPalette* OutPalette);
	bool ParseSwatch(int32 SwatchID, UConfigPropertySection* Swatch, int32 GroupID, FString GroupName, float GroupPriority, FUSSPalette* OutPalette);
	FLinearColor HexToLinearColor(FString HexCode);
	UTexture2D* GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor);
	void RetrieveFreeColorSlotID();
};
