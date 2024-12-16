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

	/* The available palettes parsed from the configuration file. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<FString, FUSSPalette> Palettes;

	/* The colection where the swatch should be added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString SessionName;

	/* The configuration that contains all the swatch to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UConfigProperty* RootConfig;

	/* Tells if More Swatch slots mod is loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool IsUsingMoreSwatchSlots;

	/* The subsystem used to load and store swatch data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	AUniversalSwatchSlotsSubsystem* USSSubsystem = nullptr;

	/* The game instance used to construct dynamic swatch descriptor and recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UUniversalSwatchSlotsGIModule* GameInstance;

protected:

	
	/* This array contains all the free color slot IDs. */
	UPROPERTY()
	TArray<int32> ValidSlotIDs;

	
	void ParseAssociations(UConfigPropertyArray* Associations);
	void ParsePalettes(UConfigPropertyArray* PalettesArr);
	int32 ParseSwatchGroup(int32 StartValidSlotID, int32 GroupID, UConfigPropertySection* SwatchGroup, FUSSPalette* OutPalette);
	bool ParseSwatch(int32 SwatchID, UConfigPropertySection* Swatch, int32 GroupID, FString GroupName, float GroupPriority, FUSSPalette* OutPalette);
};
