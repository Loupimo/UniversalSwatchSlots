// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"

#include "UniversalSwatchSlotsDefinitions.h"

#include "UniversalSwatchSlotsGIModule.generated.h"

USTRUCT(BlueprintType)
struct FUSSSwatchGenInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UClass* SwatchClass;
};


USTRUCT(BlueprintType)
struct FUSSSwatchGroupGenInfo : public FUSSSwatchGenInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchGroup* SwatchCDO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchGroup* SwatchInst;
};


USTRUCT(BlueprintType)
struct FUSSSwatchDescGenInfo : public FUSSSwatchGenInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchDesc* SwatchCDO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchDesc* SwatchInst;
};

USTRUCT(BlueprintType)
struct FUSSSwatchRecipeGenInfo : public FUSSSwatchGenInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchRecipe* SwatchCDO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	UUSSSwatchRecipe* SwatchInst;
};

/**
 *
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUniversalSwatchSlotsGIModule : public UGameInstanceModule
{
	GENERATED_BODY()

	/**
	 * Create a new swatch descriptor and store it in the SwatchDescriptorArray.
	 *
	 * @param	UniqueID	The slot ID used by swatch descriptor. Must be unique
	 *
	 * @warning Be sure that the desired swatch descriptor doesn't exist otherwise its CDO will be modified.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void GenerateDynamicSwatchDescriptor(int32 UniqueID);

	/**
	 * Create up to 255 swatch class descriptors in order to have them ready before the world loads to avoid crash / being able to patch building that used missing swatches or from previous version.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void GenerateDynamicSwatchClasses();

public:

	/* The list of all generated swatch descriptors. This array is modified when the GenerateDynamicSwatchDescriptor function is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, FUSSSwatchDescGenInfo> SwatchDescriptorArray;

	/* The list of all generated swatch group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, FUSSSwatchGroupGenInfo> SwatchGroupArray;

	/* The list of all generated swatch recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swatch")
	TMap<int32, FUSSSwatchRecipeGenInfo> SwatchRecipeArray;

	/* Store the descriptors of previous version in order to correctly patch buildings. */
	UPROPERTY()
	TArray<UClass*> tmpSwatchDescriptorArray;

protected:

	FString PackageName = "/UniversalSwatchSlots";
};
