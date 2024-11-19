// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

#include "Module/GameWorldModule.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "FGFactoryColoringTypes.h"
#include "FGCustomizationRecipe.h"
#include "FGCustomizerSubCategory.h"
#include "FGBuildGun.h"

#include "ModConfiguration.h"
#include "ConfigPropertyArray.h"

#include "UniversalSwatchSlotsWorldModule.generated.h"

USTRUCT(BlueprintType)
struct FUSSSwatchInformation {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch> mSwatch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor mPrimaryColour = FLinearColor(250, 149, 73, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor mSecondaryColour = FLinearColor(95, 102, 140, 255);
};


USTRUCT(BlueprintType)
struct FUSSSwatch {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueGroupID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText GroupDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GroupPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SwatchSlotID = 28;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SwatchUniqueID = 28;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SwatchDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SwatchPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColour = FLinearColor(250, 149, 73, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColour = FLinearColor(95, 102, 140, 255);
};

USTRUCT(BlueprintType)
struct FUSSSession {
	GENERATED_BODY()

	/* The palette name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;

	/* Tells if the primary colors should be added to the player clor preset or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AddPrimaryColorsToPreset;

	/* Tells if the secondary colors should be added to the player clor preset or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AddSecondaryColorsToPreset;
};

USTRUCT(BlueprintType)
struct FUSSPalette {
	GENERATED_BODY()

	/* The palette name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PaletteName;

	/* The associated session to this palette. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <FUSSSession> AssociatedSessions;

	/* The swatches contained in this palette. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <FUSSSwatch> Swatches;
};

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
	void AddNewSwatchesColorSlots(TArray<FUSSSwatchInformation> SwatchInformations);

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
	UFGCustomizerSubCategory* GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority);

	/**
	 * Create a new swatch descriptor using the desired swatch group, name and ID.
	 *
	 * @param	SlotID					The slot ID used by swatch descriptor.
	 * @param	UniqueID				The unique ID to give to the swatch descriptor. Used to create its identifier. It is the internal ID of this mod and can be different from Slot ID if other mod add swatches.
	 * @param	DisplayName				The name to give to the swatch descriptor.
	 * @param	Priority				The priority to give to this swatch.
	 * @param   PrimaryColor			The primary color used to generate the swatch icon.
	 * @param	SecondaryColor			The secondary color used to generate the swatch icon.
	 * @param	SwatchGroup				The swatch group to use.
	 *
	 * @warning Be sure that the desired swatch descriptor doesn't exist otherwise its CDO will be modified.
	 *
	 * @return The newly generated swatch descriptor, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UFGFactoryCustomizationDescriptor_Swatch* GenerateDynamicSwatchDescriptor(int32 SlotID, int32 UniqueID, FText DisplayName, float Priority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory* SwatchGroup);

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
	UFGCustomizationRecipe* GenerateDynamicSwatchRecipe(UFGFactoryCustomizationDescriptor_Swatch* SwatchDescriptor);

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
	bool GenerateNewSwatch(int32 UniqueGroupID, FText GroupDisplayName, float GroupPriority, int32 SlotID, int32 SwatchUniqueID, FText SwatchDisplayName, float SwatchPriority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe);

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
	bool GenerateNewSwatchUsingInfo(FUSSSwatch SwatchInformation, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe);

	/**
	 * Parse the mod's configuration referenced by the variable ModConfig.
	 * 
	 * @return True if the has been correctly parsed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	bool ParseModConfig();


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
	TMap<int32, UFGCustomizerSubCategory*> SwatchGroupArray;

	/* The list of all generated swatch descriptors. This array is modified when the GenerateDynamicSwatchDescriptor function is called.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, UFGFactoryCustomizationDescriptor_Swatch*> SwatchDescriptorArray;

	/* The build gun blueprint class that will be used by the dynamically created swatch recipes. This array is modified when the GenerateDynamicSwatchRecipe function is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, UFGCustomizationRecipe*> SwatchRecipeArray;


	/* The image used as template to generate swatch icon for the awesome shop. */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UTexture2D* SwatchTemplate;*/


	/* The colection where the swatch should be added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SessionName;

	/* The configuration that contains all the swatch to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UConfigProperty* RootConfig;

	/* The name of the current session. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UFGFactoryCustomizationCollection* SwatchCollection;

	/* The available palettes parsed from the configuration file. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TMap<FString, FUSSPalette> Palettes;

protected:

	UPROPERTY()
	TArray<UClass*> GeneratedClasses;

	UClass* GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName);

	
	void ParseAssociations(UConfigPropertyArray* Associations);
	void ParsePalettes(UConfigPropertyArray* PalettesArr);
	int32 ParseSwatchGroup(int32 GroupID, int32* SlotID, int32 StartSwatchID, UConfigPropertySection* SwatchGroup, FUSSPalette* OutPalette);
	bool ParseSwatch(int32 SlotID, int32 SwatchID, UConfigPropertySection* Swatch, int32 GroupID, FString GroupName, float GroupPriority, FUSSPalette* OutPalette);
	FLinearColor HexToLinearColor(FString HexCode);
	UTexture2D* GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor);
	int32 FindUSSStartSlotID();
};
