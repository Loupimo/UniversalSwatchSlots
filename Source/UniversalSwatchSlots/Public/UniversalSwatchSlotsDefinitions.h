// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGFactoryColoringTypes.h"
#include "FGCustomizationRecipe.h"
#include "FGCustomizerSubCategory.h"

#include "UniversalSwatchSlotsDefinitions.generated.h"


USTRUCT(BlueprintType)
struct FUSSSwatchSaveInfo
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	int32 SwatchSlotID;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FText SwatchDisplayName;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FString SwatchGeneratedName;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColour = FLinearColor(250, 149, 73, 255);

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColour = FLinearColor(95, 102, 140, 255);
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
	int32 SwatchUniqueID = 0;

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
class UNIVERSALSWATCHSLOTS_API UUSSSwatchGroup : public UFGCustomizerSubCategory
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString HashName;
};


/**
 *
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSSwatchDesc : public UFGFactoryCustomizationDescriptor_Swatch
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString HashName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor PrimaryColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor SecondaryColour;
};


/**
 *
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSSwatchRecipe : public UFGCustomizationRecipe
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString HashName;
};

