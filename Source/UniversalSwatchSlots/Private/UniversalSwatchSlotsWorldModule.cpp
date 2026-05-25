// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
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

TArray<FGlobalColorPreset> UUniversalSwatchSlotsWorldModule::ApplySwatchesColorOptionToPreset(TArray<UUSSSwatchDesc*> SwatchDescriptions, bool AddPrimaryColors, bool AddSecondaryColors, bool RemovePrimayColors, bool RemoveSecondaryColors)
{
    TArray<FGlobalColorPreset> FinalColorPreset;

    AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));
    if (!FGGameState)
    {
        return FinalColorPreset;
    }

    if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
    {
        return FinalColorPreset;
    }

    if (!AddPrimaryColors && !AddSecondaryColors && !RemovePrimayColors && !RemoveSecondaryColors)
    {
        return FinalColorPreset;
    }

    TArray<FString> ColorToRemove;

    for (UUSSSwatchDesc* CurrSwatch : SwatchDescriptions)
    {
        if (!CurrSwatch)
        {
            continue;
        }

        const FString PrimName = CurrSwatch->mDisplayName.ToString() + TEXT(" - P");
        const FString SecName = CurrSwatch->mDisplayName.ToString() + TEXT(" - S");

        //
        // PRIMARY
        //
        if (AddPrimaryColors)
        {
            //
            // Replace existing if present
            //
            ColorToRemove.Add(PrimName);

            FinalColorPreset.Add(FGlobalColorPreset(FText::FromString(PrimName), CurrSwatch->PrimaryColour));
        }
        else if (RemovePrimayColors)
        {
            ColorToRemove.Add(PrimName);
        }

        //
        // SECONDARY
        //
        if (AddSecondaryColors)
        {
            //
            // Replace existing if present
            //
            ColorToRemove.Add(SecName);

            FinalColorPreset.Add(FGlobalColorPreset(FText::FromString(SecName), CurrSwatch->SecondaryColour));
        }
        else if (RemoveSecondaryColors)
        {
            ColorToRemove.Add(SecName);
        }
    }

    //
    // Keep existing presets that are NOT removed
    //
    for (const FGlobalColorPreset& ExistingPreset : FGGameState->mPlayerGlobalColorPresets)
    {
        const FString ExistingName = ExistingPreset.PresetName.ToString();

        if (!ColorToRemove.Contains(ExistingName))
        {
            FinalColorPreset.Add(ExistingPreset);
        }
    }

    //
    // Remove presets
    // Reverse iteration mandatory
    //
    for (int32 i = FGGameState->mPlayerGlobalColorPresets.Num() - 1; i >= 0; --i)
    {
        FGGameState->RemovePlayerColorPresetAtIndex(i);
    }

    // Sort by swatch name
    FinalColorPreset.Sort([](const FGlobalColorPreset& Lhs, const FGlobalColorPreset& Rhs) -> bool {
        if (Lhs.PresetName.ToString() < Rhs.PresetName.ToString()) return true;
        return false;
        });

    return FinalColorPreset;
}
