// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGFactoryColoringTypes.h"
#include "FGCustomizationRecipe.h"
#include "FGCustomizerSubCategory.h"

#include "UniversalSwatchSlotsDefinitions.generated.h"

UENUM(BlueprintType)
enum class EUSSVersion : uint8
{
	None  = 0	UMETA(DisplayName = "None"),		// Not set or before 1.0.4
	V1_0_4 = 1	UMETA(DisplayName = "1.0.4")		// 1.0.4
};

UENUM(BlueprintType)
enum class EUSSSwatchMaterial : uint8
{
	Default = 0	UMETA(DisplayName = "Default"),		// Default material
	Matte = 1	UMETA(DisplayName = "Matte"),		// Matte material
	Shiny = 2	UMETA(DisplayName = "Shiny"),		// Shiny material
};

USTRUCT(BlueprintType)
struct FUSSSwatchSaveInfo
{
	GENERATED_BODY()

public:

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

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Priority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUSSSwatchMaterial Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PrimaryColor;
	//FLinearColor PrimaryColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SecondaryColor;
	//FLinearColor SecondaryColour;
};

USTRUCT(BlueprintType)
struct FUSSSession {
	GENERATED_BODY()

public:
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
struct FUSSGroup {
	GENERATED_BODY()

public:
	/* The palette name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/* The palette name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Priority;

	/* The swatches contained in this palette. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <FUSSSwatch> Swatches;
};

USTRUCT(BlueprintType)
struct FUSSPalette : public FTableRowBase {
	GENERATED_BODY()

public:
	/* The palette name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PaletteName;

	/* Tells if this palette is active and should be loaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsActive;

	/* Tells if the primary colors should be added to the player color preset or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AddPrimaryColorsToPreset;

	/* Tells if the secondary colors should be added to the player color preset or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AddSecondaryColorsToPreset;

	/* The swatches contained in this palette. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray <FUSSGroup> SwatchGroups;
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

