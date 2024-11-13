// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "FGFactoryColoringTypes.h"
#include "FGCustomizationRecipe.h"
#include "FGCustomizerSubCategory.h"
#include "FGBuildGun.h"

#include "USSCustomizerInterface.h"
#include "UniversalSwatchSlotsWorldModule.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUniversalSwatchSlotsWorldModule : public UGameWorldModule
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void AddNewSwatches(TArray<FUSSSwatchInformation> SwatchInformations);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UFGCustomizerSubCategory* GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UFGFactoryCustomizationDescriptor_Swatch* GenerateDynamicSwatchDescriptor(int32 UniqueID, FText DisplayName, UFGCustomizerSubCategory* SwatchGroup);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	UFGCustomizationRecipe* GenerateDynamicSwatchRecipe(UFGFactoryCustomizationDescriptor_Swatch* SwatchDescriptor);


public:

	UPROPERTY(EditDefaultsOnly, Category = "Recipe reference")
	TSoftClassPtr<UObject> BuildGunBPClass;
};
