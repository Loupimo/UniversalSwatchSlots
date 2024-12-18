// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsSubsystem.h"

#include "Subsystem/SubsystemActorManager.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "USSBPLib.h"

const EUSSVersion CurrVersion = EUSSVersion::V1_0_4;
const FString USSSubPackageName = "/UniversalSwatchSlots";

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


UUSSSwatchGroup* AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority)
{
	if (this->SwatchGroupArray.Contains(UniqueGroupID))
	{	// The group already exist

		UUSSSwatchGroup* SGCDO = Cast<UUSSSwatchGroup>((*this->SwatchGroupArray.Find(UniqueGroupID))->GetClass()->GetDefaultObject());
		if (SGCDO)
		{	// Overwrite the group name and priority

			SGCDO->mDisplayName = DisplayName;
			SGCDO->mMenuPriority = Priority;
		}

		return *this->SwatchGroupArray.Find(UniqueGroupID);
	}
	else
	{
		FString genName = FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID);

		// Create a dynamic derivated class
		UClass* NewClass = (UClass*)UUSSBPLib::FindOrCreateClass(*USSSubPackageName, genName, UUSSSwatchGroup::StaticClass());

		if (NewClass)
		{
			UObject* tempCDO = NewClass->GetDefaultObject();

			if (tempCDO)
			{
				// Modify CDO properties of the newly generated class
				UUSSSwatchGroup* CDO = Cast<UUSSSwatchGroup>(tempCDO);

				if (CDO)
				{
					CDO->mDisplayName = DisplayName;
					CDO->HashName = genName;
					CDO->mMenuPriority = Priority;
				}

				// Create an instance to return
				UUSSSwatchGroup* InstClass = NewObject<UUSSSwatchGroup>(FindPackage(nullptr, *USSSubPackageName), NewClass, FName(*FString::Printf(TEXT("Inst_Gen_USS_SwatchGroup_%d"), UniqueGroupID)));

				if (InstClass)
				{	// For unknown reason this instance is not initialized using our modified CDO...

					InstClass->mDisplayName = DisplayName;
					InstClass->HashName = genName;
					InstClass->mMenuPriority = Priority;

					this->SwatchGroupArray.Add(UniqueGroupID, InstClass);
				}

				return InstClass;
			}
		}
	}

	return nullptr;
}


UUSSSwatchDesc* AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchDescriptor(int32 SlotID, FText DisplayName, FString GenName, float Priority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory* SwatchGroup)
{
	FString genName = FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), SlotID);
	UClass* NewClass = (UClass*)UUSSBPLib::FindOrCreateClass(USSSubPackageName, genName, UUSSSwatchDesc::StaticClass());;

	if (NewClass)
	{
		UObject* tCDO = NewClass->GetDefaultObject();

		if (tCDO)
		{
			// Create an instance to return
			UUSSSwatchDesc* InstClass = NewObject<UUSSSwatchDesc>(FindPackage(nullptr, *USSSubPackageName), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *genName)));


			// Modify CDO properties of the newly generated class
			UUSSSwatchDesc* CDO = Cast<UUSSSwatchDesc>(tCDO);

			if (CDO)
			{	// Modify the CDO

				CDO->ID = SlotID;
				CDO->HashName = GenName;
				CDO->PrimaryColour = PrimaryColor;
				CDO->SecondaryColour = SecondaryColor;
				CDO->mDisplayName = DisplayName;
				CDO->mDescription = this->SwatchDescription;
				CDO->mIcon = UUSSBPLib::GenerateSwatchIcon(PrimaryColor, SecondaryColor);
				CDO->mPersistentBigIcon = CDO->mIcon.Get();
				CDO->mSmallIcon = CDO->mIcon.Get();
				CDO->mCategory = this->SwatchCategory;
				CDO->mMenuPriority = Priority;
				if (SwatchGroup != nullptr) CDO->mSubCategories.Add(SwatchGroup->GetClass());
			}

			if (InstClass)
			{	// For unknown reason this instance is not initialized using our modified CDO...

				InstClass->ID = SlotID;
				InstClass->HashName = GenName;
				InstClass->PrimaryColour = PrimaryColor;
				InstClass->SecondaryColour = SecondaryColor;
				InstClass->mDisplayName = DisplayName;
				InstClass->mDescription = this->SwatchDescription;
				InstClass->mIcon = CDO->mIcon;
				InstClass->mPersistentBigIcon = CDO->mPersistentBigIcon;
				InstClass->mSmallIcon = CDO->mSmallIcon;
				InstClass->mCategory = this->SwatchCategory;
				InstClass->mMenuPriority = Priority;
				if (SwatchGroup != nullptr) InstClass->mSubCategories.Add(SwatchGroup->GetClass());
			}

			this->SwatchDescriptorArray.Add(SlotID, InstClass);
			return InstClass;
		}
	}

	return nullptr;
}


UUSSSwatchRecipe* AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchRecipe(int32 UniqueID, UUSSSwatchDesc* SwatchDescriptor)
{
	if (SwatchDescriptor == nullptr)
	{
		return nullptr;
	}

	UUSSSwatchDesc* SwatchDescCDO = (UUSSSwatchDesc*)SwatchDescriptor->GetClass()->GetDefaultObject();

	// Create a dynamic derivated class
	FString genName = FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), UniqueID);
	UClass* NewClass = (UClass*)UUSSBPLib::FindOrCreateClass(USSSubPackageName, genName, UUSSSwatchRecipe::StaticClass());

	if (NewClass)
	{
		UObject* tempCDO = NewClass->GetDefaultObject();

		if (tempCDO)
		{
			// Modify CDO properties of the newly generated class
			UUSSSwatchRecipe* CDO = Cast<UUSSSwatchRecipe>(tempCDO);
			if (CDO)
			{
				CDO->mDisplayName = SwatchDescCDO->mDisplayName;
				CDO->HashName = genName;
				CDO->mCustomizationDesc = SwatchDescCDO->GetClass();
				CDO->mProducedIn.Add(this->BuildGunBPClass);
			}

			// Create an instance to return
			UUSSSwatchRecipe* InstClass = NewObject<UUSSSwatchRecipe>(FindPackage(nullptr, *USSSubPackageName), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *genName)));

			if (InstClass)
			{	// For unknown reason this instance is not initialized using our modified CDO...

				InstClass->mDisplayName = SwatchDescCDO->mDisplayName;
				InstClass->HashName = genName;
				InstClass->mCustomizationDesc = SwatchDescCDO->GetClass();
				InstClass->mProducedIn.Add(this->BuildGunBPClass);

				if (!this->SwatchRecipeArray.Contains(SwatchDescCDO->ID))
				{
					this->SwatchRecipeArray.Add(SwatchDescCDO->ID, InstClass);
				}
			}

			return InstClass;
		}
	}

	return nullptr;
}


bool AUniversalSwatchSlotsSubsystem::GenerateNewSwatchUsingInfo(FUSSSwatch SwatchInformation, UUSSSwatchGroup*& SwatchGroup, UUSSSwatchDesc*& SwatchDescriptor, UUSSSwatchRecipe*& SwatchRecipe)
{
	int32 slotID = this->ValidSlotIDs[0];

	if (this->SwatchDescriptorArray.Contains(slotID) || this->SwatchRecipeArray.Contains(slotID))
	{	// We can't overwrite existing swatch. (Well we could but I don't want to in order to not mess up evrything)

		SwatchGroup = nullptr;
		SwatchDescriptor = nullptr;
		SwatchRecipe = nullptr;
		return false;
	}

	//SwatchGroup = this->GenerateDynamicSwatchGroup(this->SwatchGroupArray.Num(), SwatchInformation.GroupDisplayName, SwatchInformation.GroupPriority);

	int32 nameCount = 0;
	int32* tmpPtr = this->SwatchNameCount.Find(SwatchInformation.Name.ToString());

	if (tmpPtr)
	{	// The name already exist in the list we need to generate a new one

		nameCount = *tmpPtr;
		*tmpPtr += 1;			// It is important to put this line after as the name count start at 1 if it already exist
	}
	else
	{	// The name doesn't exist yet

		this->SwatchNameCount.Add(SwatchInformation.Name.ToString(), 1);
	}

	FString genName = UUSSBPLib::BuildSwatchGenName(SwatchInformation.Name.ToString(), "SD", nameCount);

	FUSSSwatchSaveInfo tmpSaved;

	// Try to find if there is a saved swatch matching this swatch
	if (this->FindSavedSwatch(genName, tmpSaved))
	{	// There is one

		this->InternalSwatchMatch.Add(tmpSaved.SwatchSlotID, slotID);
		UE_LOG(LogUSS_Subsystem, Verbose, TEXT("Found saved color \"%s\" at slot : %d"), *genName, tmpSaved.SwatchSlotID);
	}
	else
	{	// There is none

		this->InternalSwatchMatch.Add(slotID, slotID);
	}

	//SwatchDescriptor = this->GenerateDynamicSwatchDescriptor(slotID, SwatchInformation.Name, genName, SwatchInformation.Priority, SwatchInformation.PrimaryColour, SwatchInformation.SecondaryColour, SwatchGroup);

	this->ValidSlotIDs.RemoveAt(0);
	//SwatchInformation.SwatchUniqueID = slotID;
	SwatchRecipe = this->GenerateDynamicSwatchRecipe(slotID, SwatchDescriptor);

	return true;
}


void AUniversalSwatchSlotsSubsystem::PatchBuildingsSwatchDescriptor()
{
	TArray<AActor*> foundActors;
	bool shouldPatch = false;

	if (this->SaveVersion == EUSSVersion::None)
	{	// We need to make a check anyway

		shouldPatch = true;
	}
	else
	{
		for (auto Match : this->InternalSwatchMatch)
		{
			if (Match.Key != Match.Value)
			{	// We need to update

				shouldPatch = true;
				break;
			}
		}
	}

	if (shouldPatch)
	{	// We should patch buildings

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildable::StaticClass(), foundActors);
		UE_LOG(LogUSS_Subsystem, Display, TEXT("Version or palette mismatch. Patching buildings descriptor..."));

		for (AActor* CurrBuilding : foundActors)
		{	// Check all actors of type AFGBuildable

			AFGBuildable* castBuild = (AFGBuildable*)CurrBuilding;

			UClass* tmpDesc = castBuild->mCustomizationData.SwatchDesc.Get();

			if (tmpDesc && tmpDesc->IsChildOf(UUSSSwatchDesc::StaticClass()))
			{	// The descriptor is from USS mod

				UUSSSwatchDesc* castDesc = (UUSSSwatchDesc*)tmpDesc;

				FString tmpStr;
				castDesc->GetName(tmpStr);

				tmpStr.RemoveFromStart("Gen_USS_SwatchDesc_");
				tmpStr.RemoveFromEnd("_C");

				int* matchID = this->InternalSwatchMatch.Find(FCString::Atoi(*tmpStr));

				if (matchID)
				{	// We have found a matching swatch descriptor 

					UUSSSwatchDesc** newDesc = this->SwatchDescriptorArray.Find(*matchID);

					if (newDesc)
					{	// There is a valid swatch descriptor

						castBuild->mCustomizationData.SwatchDesc = (*newDesc)->GetClass()->GetDefaultObject()->GetClass();
						UE_LOG(LogUSS_Subsystem, Display, TEXT("Patching descriptor \"%s\" with \"%s\" named \"%s\" for building \"%s\"."), *castDesc->GetPathName(), *((*newDesc)->GetClass()->GetDefaultObject()->GetClass()->GetPathName()), *(*newDesc)->mDisplayName.ToString(), *castBuild->GetName());
					}
					else
					{	// There is no valid descriptor

						UE_LOG(LogUSS_Subsystem, Display, TEXT("Can't patch \"%s\" for building \"%s\". No matching descriptor found"), *castDesc->GetPathName(), *castBuild->GetName());
					}
				}
			}
		}
	}
}


void AUniversalSwatchSlotsSubsystem::RetrieveFreeColorSlotID()
{
	int32 SFSlots = 28; // 0 - 17 swatches, 18 = concrete, 19 = carbon, 20 = caterium, 21 = chrome, 22 = copper, 23 = unpainted, 24 - 27 = project assembly

	int32 StartID = SFSlots;

	if (this->IsUsingMSS)
	{	// At the moment MSS adds 20 pre-defined index for swatches (28 - 47)

		StartID += 20;
	}

	// 255 = custom
	for (int32 i = StartID; i < 255; i++)
	{
		this->ValidSlotIDs.Add(i);
	}
}


bool AUniversalSwatchSlotsSubsystem::FindSavedSwatch(FString GeneratedName, FUSSSwatchSaveInfo& Out)
{
	for (int32 i = 0; i < this->SavedSwatches.Num(); i++)
	{
		FUSSSwatchSaveInfo currSaved = this->SavedSwatches[i];
		if (currSaved.SwatchGeneratedName.Equals(GeneratedName))
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
		
		uint32 matchingID = currSaved.SwatchSlotID;

		if (this->WasUsingMSS)
		{	// The save file was using MSS

			if (!this->IsUsingMSS)
			{	// MSS is not currently loaded 

				matchingID -= 20;

				UE_LOG(LogUSS_Subsystem, Warning, TEXT("More swatch slot was used by the saved game but is not currently installed. Fixing index."));
			}
		}
		else
		{	// The save file was not using MSS

			if (this->IsUsingMSS)
			{ // MSS is loaded

				matchingID += 20;

				UE_LOG(LogUSS_Subsystem, Warning, TEXT("More swatch slot wasn't used by the saved game but is currently installed. Fixing index."));
			}
		}

		UUSSSwatchDesc** tmp = this->SwatchDescriptorArray.Find(matchingID);

		if (tmp)
		{	// We have found a match

			this->InternalSwatchMatch.Add(currSaved.SwatchSlotID, (*tmp)->ID);
			UE_LOG(LogUSS_Subsystem, Warning, TEXT("Found color \"%s\" with slotID %d in saved game but no matching palette name. Replacing with \"%s\""), *currSaved.SwatchDisplayName.ToString(), currSaved.SwatchSlotID, *(*tmp)->mDisplayName.ToString());
		}
		else
		{	// We haven't found a match

			UE_LOG(LogUSS_Subsystem, Warning, TEXT("Found existing color \"%s\" with slotID %d but no matching swatch in the given palette. WARNING: if you save your game this color will be removed or replaced with the new swatch that has the same slot ID if any."), *currSaved.SwatchDisplayName.ToString(), currSaved.SwatchSlotID);
		}
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


void AUniversalSwatchSlotsSubsystem::UpdateSavedVersion()
{
	this->SaveVersion = CurrVersion;
}