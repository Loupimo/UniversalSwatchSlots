// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "UniversalSwatchSlots.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "FGGameState.h"

#include "USSBuildGunPaintMode.h"
#include "USSSameSwatchPaintMode.h"
#include "FGFactoryColoringTypes.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunPaint.h"
#include "Buildables/FGBuildable.h"
#include "FGBlueprintProxy.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"

#if PLATFORM_WINDOWS
#include <excpt.h>
#endif

namespace
{
	// Isolated so MSVC __try can catch AVs from corrupted FText shared data.
	static void USS_FTextToStringUnsafe(const FText* Text, FString* OutString)
	{
        *OutString = Text->ToString();
	}

	/**
	 * FText::ToString() / Rebuild() can ACCESS_VIOLATION on corrupted or stale preset
	 * names (legacy saves, mod conflicts, invalid localization data). Never call ToString
	 * unguarded on player color presets loaded from game state.
	 */
	static bool USS_TryFTextToString(const FText& Text, FString& OutString)
	{
		bool bOk = false;

#if PLATFORM_WINDOWS
		__try
		{
			USS_FTextToStringUnsafe(&Text, &OutString);
			bOk = true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bOk = false;
		}
#else
		OutString = Text.ToString();
		bOk = true;
#endif

		if (!bOk)
		{
			OutString.Reset();
		}

		return bOk;
	}

	static FString USS_ResolveSwatchDisplayName(const FText& DisplayNameText, const FString& HashNameFallback)
	{
		FString DisplayName;
		if (USS_TryFTextToString(DisplayNameText, DisplayName) && !DisplayName.IsEmpty())
		{
			return DisplayName;
		}

		// Culture-invariant fallback when mDisplayName is empty or unreadable.
		return HashNameFallback;
	}
}

void UUniversalSwatchSlotsWorldModule::GenerateSwatchesFromPalette(FUSSPalette Palette)
{
	this->USSSubsystem->GeneratePalette(Palette);
}


void UUniversalSwatchSlotsWorldModule::InitUSSGameWorldModule(UUniversalSwatchSlotsGIModule* USSInstance, AUniversalSwatchSlotsSubsystem* Subsystem)
{
	this->USSSubsystem = Subsystem;
	this->USSSubsystem->USSInst = USSInstance;
	this->USSSubsystem->IsUsingSwatchMods = this->IsUsingSwatchMods;
    this->USSSubsystem->ExternalAddedSwatches = this->ExternalAddedSwatches;
	this->USSSubsystem->RetrieveFreeColorSlotID();
}

void UUniversalSwatchSlotsWorldModule::GetBlueprintPaintCost(AFGBuildGun* BuildGun, TArray<FItemAmount>& OutCost, int32& OutBuildingCount) const
{
	OutCost.Reset();
	OutBuildingCount = 0;

	if (!BuildGun)
	{
		return;
	}

	// Only meaningful while the build gun is in the Paint state.
	UFGBuildGunStatePaint* paint = Cast<UFGBuildGunStatePaint>(BuildGun->GetCurrentState());
	if (!paint)
	{
		return;
	}

	// No-build-cost (creative): nothing to display.
	const AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(BuildGun->GetInstigator());
	const UFGInventoryComponent* inventory = character ? character->GetInventory() : nullptr;
	if (inventory && inventory->GetNoBuildCost())
	{
		return;
	}

	// Resolve the same building the paint will act on (handles the material preview actor too).
	AFGBuildable* aimed = FUSSBuildGunPaintMode::ResolveAimedBuildable(paint, BuildGun->GetHitResult().GetActor());
	AFGBlueprintProxy* proxy = aimed ? aimed->GetBlueprintProxy() : nullptr;
	if (!proxy)
	{
		return; // not aiming at a blueprint
	}

	OutBuildingCount = FUSSBuildGunPaintMode::CountPlanBuildings(proxy);
	OutCost = FUSSBuildGunPaintMode::ScaleCost(paint->GetCustomizationCost(), OutBuildingCount);
}

bool UUniversalSwatchSlotsWorldModule::GetClientPreview() const
{
    return this->IsClientPreviewEnabled;
}

void UUniversalSwatchSlotsWorldModule::ToggleSameSwatchLock()
{
    FUSSSameSwatchPaintMode::ToggleLock();
}

void UUniversalSwatchSlotsWorldModule::GetSameSwatchInfo(TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch>& OutSwatchDesc, int32& OutCount, bool& OutLocked) const
{
    OutSwatchDesc = FUSSSameSwatchPaintMode::GetReferenceSwatch();
    OutCount = FUSSSameSwatchPaintMode::GetMatchCount();
    OutLocked = FUSSSameSwatchPaintMode::IsLocked();
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

        const FString BaseName = USS_ResolveSwatchDisplayName(CurrSwatch->mDisplayName, CurrSwatch->HashName);
        if (BaseName.IsEmpty())
        {
            UE_LOG(LogUSS, Warning, TEXT("ApplySwatchesColorOptionToPreset: skipping swatch with unreadable display name."));
            continue;
        }

        const FString PrimName = BaseName + TEXT(" - P");
        const FString SecName = BaseName + TEXT(" - S");

        //
        // PRIMARY
        //
        if (AddPrimaryColors)
        {
            //
            // Replace existing if present
            //
            ColorToRemove.Add(PrimName);

            // FromString creates culture-invariant text (stable across language / I18N rebuilds).
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
    // Keep existing presets that are NOT removed.
    // PresetName.ToString() must be guarded: corrupted FText in mPlayerGlobalColorPresets
    // causes EXCEPTION_ACCESS_VIOLATION inside FText::Rebuild (seen in shipping crash dumps).
    //
    for (const FGlobalColorPreset& ExistingPreset : FGGameState->mPlayerGlobalColorPresets)
    {
        FString ExistingName;
        if (!USS_TryFTextToString(ExistingPreset.PresetName, ExistingName))
        {
            UE_LOG(LogUSS, Warning, TEXT("ApplySwatchesColorOptionToPreset: dropping color preset with corrupted PresetName."));
            continue;
        }

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

    // Sort by swatch name (guarded — same corruption risk as above)
    FinalColorPreset.Sort([](const FGlobalColorPreset& Lhs, const FGlobalColorPreset& Rhs) -> bool {
        FString LhsName;
        FString RhsName;
        USS_TryFTextToString(Lhs.PresetName, LhsName);
        USS_TryFTextToString(Rhs.PresetName, RhsName);
        return LhsName < RhsName;
        });

    return FinalColorPreset;
}
