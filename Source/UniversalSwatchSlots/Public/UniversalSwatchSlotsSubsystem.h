// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FGFactoryColoringTypes.h"
#include "UniversalSwatchSlotsSubsystem.generated.h"

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

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API AUniversalSwatchSlotsSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "Subsystem", DisplayName = "GetAUniversalSwatchSlotsSubsystem", meta = (DefaultToSelf = "WorldContext"))
	static AUniversalSwatchSlotsSubsystem* Get(UObject* worldContext);

	/**
	 * Find the matching saved swatch if any.
	 *
	 *
	 * @param	SwatchDisplayName		The swatch display name.
	 * @param	SwatchClassAcr			The swatch class acronym.
	 * @param	SwatchID				The swatch ID. It should match the occurrence number of the swatch display name you are trying to find (starting at index 0). 
	 *
	 * @return The matching generated name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	static FString BuildSwatchGenName(FString SwatchDisplayName, FString SwatchClassAcr, int32 SwatchID);

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
	void UpdateSavedSwatches(TArray<FUSSSwatchInformation> ToSave);

public:

	/* A detailed array that contains info about saved swatches. */
	UPROPERTY(SaveGame, BlueprintReadWrite)
	TArray<FUSSSwatchSaveInfo> SavedSwatches;
};
