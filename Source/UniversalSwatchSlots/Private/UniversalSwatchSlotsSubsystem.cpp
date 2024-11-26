// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsSubsystem.h"

#include "Net/UnrealNetwork.h"
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

		if (SubsystemActorManager->SubsystemActors.IsEmpty())
		{	// The subsytem is replicated

			FLatentActionInfo dummy;
			dummy.CallbackTarget = AUniversalSwatchSlotsSubsystem::StaticClass();
			dummy.UUID = 123456789;
			SubsystemActorManager->WaitForSubsystem(AUniversalSwatchSlotsSubsystem::StaticClass(), dummy);

		/*	for (auto& Subsystem : SubsystemActorManager->RegisteredSubsystems)
			{
				if (Subsystem->IsChildOf(AUniversalSwatchSlotsSubsystem::StaticClass()))
				{
					return (AUniversalSwatchSlotsSubsystem*)(&Subsystem);
				}
			}*/
		}
		//else
		{	// The subsystem is spawned

			for (auto Subsystem : SubsystemActorManager->SubsystemActors)
			{
				if (Subsystem.Key->IsChildOf(AUniversalSwatchSlotsSubsystem::StaticClass()))
				{
					return Cast<AUniversalSwatchSlotsSubsystem>(Subsystem.Value);
				}
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

void AUniversalSwatchSlotsSubsystem::UpdateSavedSwatches(TArray<UUSSSwatchDesc*> ToSave)
{
	for (FUSSSwatchSaveInfo& currSaved : this->SavedSwatches)
	{	// Print a warning for all remaining swatches that were not found

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

void AUniversalSwatchSlotsSubsystem::UpdateSwatchesArray(TMap<int32, UUSSSwatchGroup*> Groups, TMap<int32, UUSSSwatchDesc*> Descs, TMap<int32, UUSSSwatchRecipe*> Recipes)
{
	TArray<int32> keys;
	Groups.GetKeys(keys);

	for (int32 currKey : keys)
	{
		this->SwatchGroupArray.Add(*Groups.Find(currKey));
	}

	Descs.GetKeys(keys);

	for (int32 currKey : keys)
	{
		this->SwatchDescriptorArray.Add(*Descs.Find(currKey));
	}

	Recipes.GetKeys(keys);

	for (int32 currKey : keys)
	{
		this->SwatchRecipeArray.Add(*Recipes.Find(currKey));
	}
}

void AUniversalSwatchSlotsSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUniversalSwatchSlotsSubsystem, SwatchGroupArray);
	DOREPLIFETIME(AUniversalSwatchSlotsSubsystem, SwatchDescriptorArray);
	DOREPLIFETIME(AUniversalSwatchSlotsSubsystem, SwatchRecipeArray);
	DOREPLIFETIME(AUniversalSwatchSlotsSubsystem, SwatchPalette);
}