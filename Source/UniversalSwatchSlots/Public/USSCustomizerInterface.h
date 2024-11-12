// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FGSwatchGroup.h"
#include "UObject/Interface.h"
#include "USSCustomizerInterface.generated.h"

USTRUCT(BlueprintType)
struct FUSSSwatchInformation {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch> mSwatch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor mPrimaryColour = FColor(250, 149, 73, 255);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor mSecondaryColour = FColor(95, 102, 140, 255);
};


USTRUCT(BlueprintType)
struct FUSSMaterialDescriptorInformation {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<class UFGFactoryCustomizationDescriptor_Material>> mApplyThisInformationTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<class AFGBuildable>> mValidBuildables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<class AFGBuildable>, TSubclassOf<class UFGRecipe>> mBuildableMap;
};


UINTERFACE()
class UNIVERSALSWATCHSLOTS_API UUSSCustomizerInterface : public UInterface
{
	GENERATED_BODY()
	
};

/**
 *
 */
class UNIVERSALSWATCHSLOTS_API IUSSCustomizerInterface {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "KMods|Customizer Interface")
	TArray<FUSSMaterialDescriptorInformation> GetMaterialInformation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "KMods|Customizer Interface")
	TArray<FUSSSwatchInformation> GetSwatchDescriptionInformation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "KMods|Customizer Interface")
	TMap<TSubclassOf<UFGSwatchGroup>, TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch>> GetSwatchGroups();
};