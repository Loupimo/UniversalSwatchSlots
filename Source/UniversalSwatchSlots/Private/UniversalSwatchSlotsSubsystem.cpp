// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsSubsystem.h"

#include "Subsystem/SubsystemActorManager.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "Kismet/GameplayStatics.h"

const EUSSVersion CurrVersion = EUSSVersion::V1_0_4;

DECLARE_LOG_CATEGORY_EXTERN(LogUSS_Subsystem, Log, All)

DEFINE_LOG_CATEGORY(LogUSS_Subsystem)

/*AUniversalSwatchSlotsSubsystem::AUniversalSwatchSlotsSubsystem()
{
	//this-> IsFirstLaunch = true;

	// Default Satisfactory color slots
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.952941, 0.301961, 0.066667, 1.000000), FLinearColor(0.113725, 0.133333, 0.262745, 1.000000))); // Ficsit
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.144128, 0.386430, 1.000000), FLinearColor(0.904661, 0.456411, 0.015996, 1.000000))); // Swatch 1
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.015996, 0.016807, 0.018500, 1.000000), FLinearColor(0.496933, 0.000000, 0.000000, 1.000000))); // Swatch 2
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.309469, 0.304987, 0.291771, 1.000000), FLinearColor(0.090842, 0.107023, 0.070360, 1.000000))); // Swatch 3
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.026241, 0.450786, 0.603828, 1.000000), FLinearColor(0.590619, 0.040915, 0.679543, 1.000000))); // Swatch 4
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.074214, 0.086500, 0.033105, 1.000000), FLinearColor(0.141263, 0.122139, 0.114435, 1.000000))); // Swatch 5
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.964687, 0.921582, 0.768151, 1.000000), FLinearColor(0.955974, 0.300544, 0.066626, 1.000000))); // Swatch 6
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.234551, 0.132868, 0.456411, 1.000000), FLinearColor(0.135633, 0.381326, 0.074214, 1.000000))); // Swatch 7
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.947307, 0.737911, 0.000000, 1.000000), FLinearColor(0.033105, 0.407240, 0.332452, 1.000000))); // Swatch 8
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.187821, 0.006995, 0.006995, 1.000000), FLinearColor(0.904661, 0.456411, 0.015996, 1.000000))); // Swatch 9
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(1.000000, 0.462077, 0.686685, 1.000000), FLinearColor(0.921582, 0.043735, 0.371238, 1.000000))); // Swatch 10
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.806952, 0.439657, 0.051269, 1.000000), FLinearColor(0.057805, 0.097587, 0.102242, 1.000000))); // Swatch 11
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.730461, 0.434154, 0.168269, 1.000000), FLinearColor(0.219526, 0.114435, 0.045186, 1.000000))); // Swatch 12
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.198069, 0.070360, 0.015209, 1.000000), FLinearColor(0.564712, 0.266356, 0.054480, 1.000000))); // Swatch 13
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.737910, 1.000000, 0.215861, 1.000000), FLinearColor(0.366253, 0.130136, 0.046665, 1.000000))); // Swatch 14
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.114435, 0.132868, 0.262251, 1.000000), FLinearColor(0.955973, 0.300544, 0.066626, 1.000000))); // Swatch 15
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.109804, 0.109804, 0.109804, 1.000000), FLinearColor(0.952941, 0.301961, 0.066667, 1.000000))); // Fondation
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.952941, 0.301961, 0.066667, 1.000000), FLinearColor(0.113725, 0.133333, 0.262745, 1.000000))); // Assembly
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(1.000000, 1.000000, 1.000000, 1.000000), FLinearColor(0.952941, 0.301961, 0.066667, 1.000000))); // Concrete

	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.000000, 0.000000, 1.000000), FLinearColor(0.000000, 0.000000, 0.000000, 1.000000))); // Carbon
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.000000, 0.000000, 1.000000), FLinearColor(0.000000, 0.000000, 0.000000, 1.000000))); // Caterium
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.000000, 0.000000, 1.000000), FLinearColor(0.000000, 0.000000, 0.000000, 1.000000))); // Chrome
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.000000, 0.000000, 1.000000), FLinearColor(0.000000, 0.000000, 0.000000, 1.000000))); // Copper
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.000000, 0.000000, 0.000000, 1.000000), FLinearColor(0.000000, 0.000000, 0.000000, 1.000000))); // Unpainted

	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.351533, 0.061246, 0.023153, 1.000000), FLinearColor(0.665387, 0.423268, 0.114435, 1.000000))); // Swatch 16
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.130136, 0.230740, 0.386430, 1.000000), FLinearColor(0.291771, 0.508881, 0.423268, 1.000000))); // Swatch 17
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.031896, 0.445201, 0.514918, 1.000000), FLinearColor(0.955974, 0.938686, 0.887923, 1.000000))); // Swatch 18
	this->DefaultSlots.Add(FFactoryCustomizationColorSlot(FLinearColor(0.952941, 0.301961, 0.066667, 1.000000), FLinearColor(0.113725, 0.133333, 0.262745, 1.000000))); // Swatch 19
}*/

AUniversalSwatchSlotsSubsystem* AUniversalSwatchSlotsSubsystem::Get(UObject* worldContext)
{
	if (worldContext)
	{
		const UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(worldContext);
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		check(SubsystemActorManager);

		for (auto Subsystem : SubsystemActorManager->SubsystemActors)
		{
			if (Subsystem.Key->IsChildOf(AUniversalSwatchSlotsSubsystem::StaticClass()))
			{
				return Cast<AUniversalSwatchSlotsSubsystem>(Subsystem.Value);
			}
		}
	}
	return nullptr;
}


FString AUniversalSwatchSlotsSubsystem::BuildSwatchGenName(FString SwatchDisplayName, FString SwatchClassAcr, int32 SwatchID)
{
	FString tmp = FString::FromInt(GetTypeHash(SwatchDisplayName));
	return FString("Gen_USS_") + SwatchClassAcr + FString("_") + tmp + FString("_") + FString::FromInt(SwatchID);
}


bool AUniversalSwatchSlotsSubsystem::FindSavedSwatch(FString GeneratedName, FUSSSwatchSaveInfo& Out)
{
	FString tmpGenName = GeneratedName.Replace(TEXT("Default__"), TEXT(""));
	for (int32 i = 0; i < this->SavedSwatches.Num(); i++)
	{
		FUSSSwatchSaveInfo currSaved = this->SavedSwatches[i];
		if (currSaved.SwatchGeneratedName.Equals(tmpGenName))
		{	// We have found the swatch. We can remove it from the array to speed up the next find

			this->SavedSwatches.RemoveAt(i);
			this->InternalSwatchMatch.Add(i);
			Out = currSaved;
			return true;
		}
	}

	return false;
}

void AUniversalSwatchSlotsSubsystem::UpdateSavedSwatches(TArray<UUSSSwatchDesc*> ToSave)
{
	for (FUSSSwatchSaveInfo& currSaved : this->SavedSwatches)
	{	// Print a warning for all remaining swatches that were not found

		if (ToSave.IsValidIndex(currSaved.SwatchSlotID))
		{	// Update the new slot with the saved hash name in order to keep the buildings colored

			UUSSSwatchDesc* CDO = (UUSSSwatchDesc*)ToSave[currSaved.SwatchSlotID]->GetClass()->GetDefaultObject();
			CDO->HashName = currSaved.SwatchGeneratedName;
			ToSave[currSaved.SwatchSlotID]->HashName = currSaved.SwatchGeneratedName;
		}
		UE_LOG(LogUSS_Subsystem, Warning, TEXT("Found existing color \"%s\" with slotID %d but no matching swatch in the given palette. WARNING: if you save your game this color will be removed or replaced with the new swatch that has the same slot ID if any."), *currSaved.SwatchDisplayName.ToString(), currSaved.SwatchSlotID);
	}

	this->SavedSwatches.Empty();

	for (UUSSSwatchDesc* Swatch : ToSave)
	{	// Browse all swatches to save

		FUSSSwatchSaveInfo newInfo;
		UUSSSwatchDesc* CDO = (UUSSSwatchDesc*)Swatch->GetClass()->GetDefaultObject();

		newInfo.PrimaryColour = Swatch->PrimaryColour;
		newInfo.SecondaryColour = Swatch->SecondaryColour;
		newInfo.SwatchSlotID = CDO->ID;
		newInfo.SwatchDisplayName = CDO->mDisplayName;
		newInfo.SwatchGeneratedName = CDO->HashName;

		this->SavedSwatches.Add(newInfo);
	}
}

void AUniversalSwatchSlotsSubsystem::AddNewSwatchesColorSlotsToGameState(TArray<UUSSSwatchDesc*> SwatchDescriptions)
{
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));
	
	if (FGGameState)
	{

		for (UUSSSwatchDesc* Swatch : SwatchDescriptions)
		{	// Browse all the swatch descriptions

			if (Swatch)
			{
				int32 ColourIndex = Swatch->ID;

				FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(FLinearColor::Black, FLinearColor::Black);
				//NewColourSlot.PaintFinish = this->PaintFinish;

				if (FGGameState->mBuildingColorSlots_Data.Num() <= ColourIndex)
				{	// We need to create some default swatch slots

					for (int32 i = FGGameState->mBuildingColorSlots_Data.Num(); i <= ColourIndex; ++i)
					{
						if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(i))
						{	// We need to add a new slot
							
							FGGameState->mBuildingColorSlots_Data.Add(NewColourSlot);
							UE_LOG(LogUSS_Subsystem, Verbose, TEXT("New color slot added to gamestate: %d"), i);
						}
					}
				}

				// Changes the colour to the desired one
				NewColourSlot.PrimaryColor = Swatch->PrimaryColour;
				NewColourSlot.SecondaryColor = Swatch->SecondaryColour;

				// Update the subsystem and game state 
				FGGameState->mBuildingColorSlots_Data[ColourIndex] = NewColourSlot;
			}
		}

		TArray<FFactoryCustomizationColorSlot> ColorSlots = FGGameState->mBuildingColorSlots_Data;
		FGGameState->SetupColorSlots_Data(ColorSlots);

		for (int32 i = 0; i < ColorSlots.Num(); i++)
		{	// Update buildings

			FFactoryCustomizationColorSlot ColorSlot = ColorSlots[i];
			FGGameState->Server_SetBuildingColorDataForSlot(i, ColorSlot);
		}

		return;
	}
}

void AUniversalSwatchSlotsSubsystem::UpdateSavedVersion()
{
	this->SaveVersion = CurrVersion;
}