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
	UUSSSwatchDesc* GenerateDynamicSwatchDescriptor(int32 UniqueID);

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
	UUSSSwatchRecipe* GenerateDynamicSwatchRecipe(int32 UniqueID);

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

	UPROPERTY()
	TArray<UClass*> GeneratedClasses;

	UClass* GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName);

	FString PackageName = "/UniversalSwatchSlots";
};
