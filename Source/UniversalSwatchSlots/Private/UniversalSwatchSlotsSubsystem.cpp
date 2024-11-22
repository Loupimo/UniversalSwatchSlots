// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsSubsystem.h"

#include "Subsystem/SubsystemActorManager.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUSS_Subsystem, Log, All)

DEFINE_LOG_CATEGORY(LogUSS_Subsystem)

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
			Out = currSaved;
			return true;
		}
	}

	return false;
}

void AUniversalSwatchSlotsSubsystem::UpdateSavedSwatches(TArray<FUSSSwatchInformation> ToSave)
{
	for (FUSSSwatchSaveInfo& currSaved : this->SavedSwatches)
	{	// Print a warning for all remaining swatches that were not found

		UE_LOG(LogUSS_Subsystem, Warning, TEXT("Found existing color \"%s\" with slotID %d but no matching swatch in the given palette. WARNING: if you save your game this color will be removed or replaced with the new swatch that has the same slot ID if any."), *currSaved.SwatchDisplayName.ToString(), currSaved.SwatchSlotID);
	}

	this->SavedSwatches.Empty();

	for (FUSSSwatchInformation& Swatch : ToSave)
	{	// Browse all swatches to save

		FUSSSwatchSaveInfo newInfo;
		UFGFactoryCustomizationDescriptor_Swatch* CDO = Swatch.mSwatch.GetDefaultObject();

		newInfo.PrimaryColour = Swatch.mPrimaryColour;
		newInfo.SecondaryColour = Swatch.mSecondaryColour;
		newInfo.SwatchSlotID = CDO->ID;
		newInfo.SwatchDisplayName = CDO->mDisplayName;
		newInfo.SwatchGeneratedName = CDO->GetName().Replace(TEXT("Default__"), TEXT(""));

		this->SavedSwatches.Add(newInfo);
	}
}