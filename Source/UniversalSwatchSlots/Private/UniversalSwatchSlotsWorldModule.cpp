// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "Kismet/GameplayStatics.h"
#include "FGGameState.h"


void UUniversalSwatchSlotsWorldModule::GenerateSwatchesFromPalette(FUSSPalette Palette)
{
	this->USSSubsystem->GeneratePalette(Palette);
}


void UUniversalSwatchSlotsWorldModule::InitUSSGameWorldModule(UUniversalSwatchSlotsGIModule* USSInstance, AUniversalSwatchSlotsSubsystem* Subsystem)
{
	this->USSSubsystem = Subsystem;
	this->USSSubsystem->USSInst = USSInstance;
	this->USSSubsystem->IsUsingMSS = this->IsUsingMoreSwatchSlots;
	this->USSSubsystem->RetrieveFreeColorSlotID();
}


void UUniversalSwatchSlotsWorldModule::ApplySwatchesColorOptionToPreset(TArray<UUSSSwatchDesc*> SwatchDescriptions, bool AddPrimaryColors, bool AddSecondaryColors, bool RemovePrimayColors, bool RemoveSecondaryColors)
{
	if (AddPrimaryColors || AddSecondaryColors || RemovePrimayColors || RemoveSecondaryColors)
	{	// At least one action has to be performed

		AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));
		if (!FGGameState)
		{
			return;
		}

		TArray<FGlobalColorPreset> FinalColorPreset;
		TArray<FString> ColorToRemove;

		for (auto currSwatch : SwatchDescriptions)
		{	// Browse all swatch descriptors

			FString currPrimName = currSwatch->mDisplayName.ToString() + " - P";
			FString currSecName = currSwatch->mDisplayName.ToString() + " - S";

			if (AddPrimaryColors)
			{
				FinalColorPreset.Add(FGlobalColorPreset(FText::FromString(currPrimName), currSwatch->PrimaryColour));
			}
			else if (RemovePrimayColors)
			{
				ColorToRemove.AddUnique(currPrimName);
			}

			if (AddSecondaryColors)
			{
				FinalColorPreset.Add(FGlobalColorPreset(FText::FromString(currSecName), currSwatch->SecondaryColour));
			}
			else if (RemoveSecondaryColors)
			{
				ColorToRemove.AddUnique(currSecName);
			}
		}

		int32 tmpID = -1;

		for (int32 i = 0; i < FGGameState->mPlayerGlobalColorPresets.Num(); i++)
		{
			FString toFind = FGGameState->mPlayerGlobalColorPresets[i].PresetName.ToString();
			if ((tmpID = ColorToRemove.Find(toFind)) != INDEX_NONE)
			{	// The color should not be added

				ColorToRemove.RemoveAt(tmpID);
			}
			else
			{	// The color is legit

				FinalColorPreset.Add(FGGameState->mPlayerGlobalColorPresets[i]);
			}
			tmpID = INDEX_NONE;
		}

		// Sort by swatch name
		FinalColorPreset.Sort([](const FGlobalColorPreset& Lhs, const FGlobalColorPreset& Rhs) -> bool {
			if (Lhs.PresetName.ToString() < Rhs.PresetName.ToString()) return true;
			return false;
		});

		FGGameState->mPlayerGlobalColorPresets = FinalColorPreset;
	}
}