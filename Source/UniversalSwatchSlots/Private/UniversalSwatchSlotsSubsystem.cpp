// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsSubsystem.h"

#include "Subsystem/SubsystemActorManager.h"
#include "FGBuildableSubsystem.h"
#include "FGBuildable.h"
#include "FGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "USSBPLib.h"

const EUSSVersion CurrVersion = EUSSVersion::V1_2_2;
const FString USSSubPackageName = "/UniversalSwatchSlots";

DECLARE_LOG_CATEGORY_EXTERN(LogUSS_Subsystem, Log, All)

DEFINE_LOG_CATEGORY(LogUSS_Subsystem)

AUniversalSwatchSlotsSubsystem* AUniversalSwatchSlotsSubsystem::Get(UObject* WorldContext)
{
	if (WorldContext)
	{
		const UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
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
	if (!FGGameState)
	{
		return;
	}

	// Slot data is replicated from the server via AFGGameState.
	// Calling Server_SetBuildingColorDataForSlot in a loop (Reliable RPC) can easily overflow the reliable buffer during join.
	if (!HasAuthority())
	{
		UE_LOG(LogUSS_Subsystem, Verbose, TEXT("AddNewSwatchesColorSlotsToGameState called on client; skipping."));
		return;
	}

	bool bAnyChanged = false;
	/*
	TArray<FFactoryCustomizationColorSlot> newArray;

	int32 ColorToCopy = 28;	// SF default slots

	if (this->IsUsingMSS)
	{
		ColorToCopy += 20;
	}

	for (int32 i = 0; i < ColorToCopy; i++)
	{	// Add the default colors to the array

		newArray.Add(FGGameState->mBuildingColorSlots_Data[i]);
	}

	for (UUSSSwatchDesc* Swatch : SwatchDescriptions)
	{	// Browse all the swatch descriptions
		if (!Swatch)
		{
			continue;
		}

		const int32 ColourIndex = Swatch->ID;

		UE_LOG(LogUSS_Subsystem, Display, TEXT("Added New color slot : %s"), *Swatch->mDisplayName.ToString());
		FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(FLinearColor::Black, FLinearColor::Black);
		NewColourSlot.PaintFinish = this->PaintFinishes[(uint8)Swatch->Material];

		for (int32 i = newArray.Num(); i <= ColourIndex; ++i)
		{
			newArray.Add(NewColourSlot);
			bAnyChanged = true;
		}
		
		// Apply desired colors
		NewColourSlot.PrimaryColor = Swatch->PrimaryColour;
		NewColourSlot.SecondaryColor = Swatch->SecondaryColour;
		newArray[ColourIndex] = NewColourSlot;

		//FGGameState->Server_SetBuildingColorDataForSlot(ColourIndex, NewColourSlot);
	}

	FGGameState->SetupColorSlots_Data(newArray);
	FGGameState->OnRep_BuildingColorSlot_Data();*/

	for (UUSSSwatchDesc* Swatch : SwatchDescriptions)
	{	// Browse all the swatch descriptions
		if (!Swatch)
		{
			continue;
		}

		const int32 ColourIndex = Swatch->ID;

		FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(FLinearColor::Black, FLinearColor::Black);
		NewColourSlot.PaintFinish = this->PaintFinishes[(uint8)Swatch->Material];

		if (FGGameState->mBuildingColorSlots_Data.Num() <= ColourIndex)
		{	// Ensure the slot array is large enough
			for (int32 i = FGGameState->mBuildingColorSlots_Data.Num(); i <= ColourIndex; ++i)
			{
				FGGameState->mBuildingColorSlots_Data.Add(NewColourSlot);
				UE_LOG(LogUSS_Subsystem, Display, TEXT("New color slot added to gamestate: %d"), i);
				bAnyChanged = true;
			}
		}

		// Apply desired colors
		NewColourSlot.PrimaryColor = Swatch->PrimaryColour;
		NewColourSlot.SecondaryColor = Swatch->SecondaryColour;

		if (FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex))
		{
			const FFactoryCustomizationColorSlot& Existing = FGGameState->mBuildingColorSlots_Data[ColourIndex];
			if (Existing.PrimaryColor != NewColourSlot.PrimaryColor || Existing.SecondaryColor != NewColourSlot.SecondaryColor || Existing.PaintFinish != NewColourSlot.PaintFinish)
			{
				UE_LOG(LogUSS_Subsystem, Display, TEXT("Index %d, Existing.P = %f, %f, %f, S = %f, %f, %f "), ColourIndex, Existing.PrimaryColor.R, Existing.PrimaryColor.B, Existing.PrimaryColor.G, Existing.SecondaryColor.R, Existing.SecondaryColor.B, Existing.SecondaryColor.G);
				UE_LOG(LogUSS_Subsystem, Display, TEXT("Index %d, New.P      = %f, %f, %f, S = %f, %f, %f "), ColourIndex, NewColourSlot.PrimaryColor.R, Existing.PrimaryColor.B, NewColourSlot.PrimaryColor.G, NewColourSlot.SecondaryColor.R, NewColourSlot.SecondaryColor.B, NewColourSlot.SecondaryColor.G);
				FGGameState->mBuildingColorSlots_Data[ColourIndex] = NewColourSlot;
				bAnyChanged = true;
			}
		}
	}

	if (bAnyChanged)
	{
		// Let GameState apply / refresh any internal state using the current slots.
		FGGameState->SetupColorSlots_Data(FGGameState->mBuildingColorSlots_Data);
	}
}


void AUniversalSwatchSlotsSubsystem::GeneratePalette(FUSSPalette Palette)
{
	UE_LOG(LogUSS_Subsystem, Display, TEXT("Generating Palette : %s"), *Palette.PaletteName.ToString());

	if (Palette.Groups.Num() <= 0)
	{	// The palette is empty

		return;
	}

	for (FUSSGroup currGroup : Palette.Groups)
	{	// Browse all groups

		if (currGroup.Swatches.Num() <= 0)
		{
			continue;
		}

		UE_LOG(LogUSS_Subsystem, Display, TEXT("Generating Group : %s"), *currGroup.Name.ToString());

		UUSSSwatchGroup* SwatchGroup = this->GenerateDynamicSwatchGroup(this->SwatchGroupArray.Num(), currGroup);

		for (FUSSSwatch currSwatch : currGroup.Swatches)
		{	// Browse all swatches

			UE_LOG(LogUSS_Subsystem, Display, TEXT("Generating Swatch : %s"), *currSwatch.Name.ToString());

			this->GenerateNewSwatchUsingInfo(SwatchGroup, currSwatch);
		}
	}
}


UUSSSwatchGroup* AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchGroup(int32 UniqueGroupID, FUSSGroup GroupInfo)
{
	if (this->SwatchGroupArray.Contains(UniqueGroupID))
	{	// The group already exist

		UUSSSwatchGroup* SGCDO = Cast<UUSSSwatchGroup>((*this->SwatchGroupArray.Find(UniqueGroupID))->GetClass()->GetDefaultObject());
		if (SGCDO)
		{	// Overwrite the group name and priority

			SGCDO->mDisplayName = GroupInfo.Name;
			SGCDO->mMenuPriority = GroupInfo.Priority;
		}

		return *this->SwatchGroupArray.Find(UniqueGroupID);
	}
	else
	{
		FString genName = FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID);
		FUSSSwatchGroupGenInfo* SwatchGroup = nullptr;
		FUSSSwatchGroupGenInfo newSwatchGroup;
		newSwatchGroup.SwatchClass = nullptr;
		newSwatchGroup.SwatchCDO = nullptr;
		newSwatchGroup.SwatchInst = nullptr;

		if ((SwatchGroup = this->USSInst->SwatchGroupArray.Find(UniqueGroupID)) == nullptr)
		{	// The group does not exist yet, we need to create it

			newSwatchGroup.SwatchClass = UUSSBPLib::CreateClass(*USSSubPackageName, genName + FString("_Class"), UUSSSwatchGroup::StaticClass());
			newSwatchGroup.SwatchCDO = Cast<UUSSSwatchGroup>(newSwatchGroup.SwatchClass->GetDefaultObject());
			this->USSInst->SwatchGroupArray.Add(UniqueGroupID, newSwatchGroup);
			SwatchGroup = &newSwatchGroup;
		}

		if (SwatchGroup->SwatchInst == nullptr)
		{	// The group instance does not exist yet or is not valid anymore

			SwatchGroup->SwatchInst = NewObject<UUSSSwatchGroup>(FindPackage(nullptr, *USSSubPackageName), SwatchGroup->SwatchClass, *FString::Printf(TEXT("%s_Inst"), *genName));
		}

		// Modify CDO properties of the generated class
		if (SwatchGroup->SwatchCDO)
		{
			SwatchGroup->SwatchCDO->mDisplayName = GroupInfo.Name;
			SwatchGroup->SwatchCDO->HashName = genName;
			SwatchGroup->SwatchCDO->mMenuPriority = (float)GroupInfo.Priority;
		}

		// Modify instance properties of the generated class
		if (SwatchGroup->SwatchInst)
		{
			SwatchGroup->SwatchInst->mDisplayName = GroupInfo.Name;
			SwatchGroup->SwatchInst->HashName = genName;
			SwatchGroup->SwatchInst->mMenuPriority = (float)GroupInfo.Priority;

			this->SwatchGroupArray.Add(UniqueGroupID, SwatchGroup->SwatchInst);
			return SwatchGroup->SwatchInst;
		}
	}

	return nullptr;
}


FUSSSwatchDescGenInfo& AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchDescriptor(int32 SlotID, FString GenName, UFGCustomizerSubCategory* SwatchGroup, FUSSSwatch SwatchInfo)
{
	FString genName = FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), SlotID);

	FUSSSwatchDescGenInfo* SwatchDesc = nullptr;
	FUSSSwatchDescGenInfo newSwatchDesc;
	newSwatchDesc.SwatchClass = nullptr;
	newSwatchDesc.SwatchCDO = nullptr;
	newSwatchDesc.SwatchInst = nullptr;

	if ((SwatchDesc = this->USSInst->SwatchDescriptorArray.Find(SlotID)) == nullptr)
	{	// The descriptor does not exist yet, we need to create it

		newSwatchDesc.SwatchClass = UUSSBPLib::CreateClass(*USSSubPackageName, genName + FString("_Class"), UUSSSwatchDesc::StaticClass());
		newSwatchDesc.SwatchCDO = Cast<UUSSSwatchDesc>(newSwatchDesc.SwatchClass->GetDefaultObject());
		this->USSInst->SwatchDescriptorArray.Add(SlotID, newSwatchDesc);
		SwatchDesc = &newSwatchDesc;
	}

	if (SwatchDesc->SwatchInst == nullptr)
	{	// The descriptor instance does not exist yet or is not valid anymore

		SwatchDesc->SwatchInst = NewObject<UUSSSwatchDesc>(FindPackage(nullptr, *USSSubPackageName), SwatchDesc->SwatchClass, *FString::Printf(TEXT("%s_Inst"), *genName));
	}

	FLinearColor PrimaryColor = UUSSBPLib::HexToLinearColor(SwatchInfo.PrimaryColor);
	FLinearColor SecondaryColor = UUSSBPLib::HexToLinearColor(SwatchInfo.SecondaryColor);

	// Modify CDO properties of the generated class
	if (SwatchDesc->SwatchCDO)
	{
		SwatchDesc->SwatchCDO->ID = SlotID;
		SwatchDesc->SwatchCDO->HashName = GenName;
		SwatchDesc->SwatchCDO->PrimaryColour = PrimaryColor;
		SwatchDesc->SwatchCDO->SecondaryColour = SecondaryColor;
		SwatchDesc->SwatchCDO->Material = SwatchInfo.Material;
		SwatchDesc->SwatchCDO->mDisplayName = SwatchInfo.Name;
		SwatchDesc->SwatchCDO->mDescription = this->SwatchDescription;
		SwatchDesc->SwatchCDO->mIcon = UUSSBPLib::GenerateSwatchIcon(PrimaryColor, SecondaryColor);
		SwatchDesc->SwatchCDO->mPersistentBigIcon = SwatchDesc->SwatchCDO->mIcon.Get();
		SwatchDesc->SwatchCDO->mSmallIcon = SwatchDesc->SwatchCDO->mIcon.Get();
		SwatchDesc->SwatchCDO->mCategory = this->SwatchCategory;
		SwatchDesc->SwatchCDO->mMenuPriority = (float)SwatchInfo.Priority;
		if (SwatchGroup != nullptr)
		{
			SwatchDesc->SwatchCDO->mSubCategories.Empty();
			SwatchDesc->SwatchCDO->mSubCategories.AddUnique(SwatchGroup->GetClass());
		}
	}

	// Modify instance properties of the generated class
	if (SwatchDesc->SwatchInst)
	{
		SwatchDesc->SwatchInst->ID = SlotID;
		SwatchDesc->SwatchInst->HashName = GenName;
		SwatchDesc->SwatchInst->PrimaryColour = PrimaryColor;
		SwatchDesc->SwatchInst->SecondaryColour = SecondaryColor;
		SwatchDesc->SwatchInst->Material = SwatchInfo.Material;
		SwatchDesc->SwatchInst->mDisplayName = SwatchInfo.Name;
		SwatchDesc->SwatchInst->mDescription = this->SwatchDescription;
		SwatchDesc->SwatchInst->mIcon = SwatchDesc->SwatchCDO->mIcon;
		SwatchDesc->SwatchInst->mPersistentBigIcon = SwatchDesc->SwatchCDO->mPersistentBigIcon;
		SwatchDesc->SwatchInst->mSmallIcon = SwatchDesc->SwatchCDO->mSmallIcon;
		SwatchDesc->SwatchInst->mCategory = this->SwatchCategory;
		SwatchDesc->SwatchInst->mMenuPriority = (float)SwatchInfo.Priority;
		if (SwatchGroup != nullptr)
		{
			SwatchDesc->SwatchInst->mSubCategories.Empty();
			SwatchDesc->SwatchInst->mSubCategories.AddUnique(SwatchGroup->GetClass());
		}
	}

	this->SwatchDescriptorArray.Add(SlotID, SwatchDesc->SwatchInst);

	return *SwatchDesc;
}


UUSSSwatchRecipe* AUniversalSwatchSlotsSubsystem::GenerateDynamicSwatchRecipe(int32 UniqueID, FUSSSwatchDescGenInfo& SwatchDescriptor)
{
	UUSSSwatchDesc* SwatchDescCDO = SwatchDescriptor.SwatchCDO;

	// Create a dynamic derivated class
	FString genName = FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), UniqueID);

	FUSSSwatchRecipeGenInfo* SwatchRecipe = nullptr;
	FUSSSwatchRecipeGenInfo newSwatchRecipe;
	newSwatchRecipe.SwatchClass = nullptr;
	newSwatchRecipe.SwatchCDO = nullptr;
	newSwatchRecipe.SwatchInst = nullptr;

	if ((SwatchRecipe = this->USSInst->SwatchRecipeArray.Find(UniqueID)) == nullptr)
	{	// The recipe does not exist yet, we need to create it

		newSwatchRecipe.SwatchClass = UUSSBPLib::CreateClass(*USSSubPackageName, genName + FString("_Class"), UUSSSwatchRecipe::StaticClass());
		newSwatchRecipe.SwatchCDO = Cast<UUSSSwatchRecipe>(newSwatchRecipe.SwatchClass->GetDefaultObject());
		this->USSInst->SwatchRecipeArray.Add(UniqueID, newSwatchRecipe);
		SwatchRecipe = &newSwatchRecipe;
	}

	if (SwatchRecipe->SwatchInst == nullptr)
	{	// The descriptor instance does not exist yet or is not valid anymore

		SwatchRecipe->SwatchInst = NewObject<UUSSSwatchRecipe>(FindPackage(nullptr, *USSSubPackageName), SwatchRecipe->SwatchClass, *FString::Printf(TEXT("%s_Inst"), *genName));
	}

	// Modify CDO properties of the generated class
	if (SwatchRecipe->SwatchCDO)
	{
		SwatchRecipe->SwatchCDO->mDisplayName = SwatchDescCDO->mDisplayName;
		SwatchRecipe->SwatchCDO->HashName = genName;
		SwatchRecipe->SwatchCDO->mCustomizationDesc = SwatchDescCDO->GetClass();
		SwatchRecipe->SwatchCDO->mProducedIn.Empty();
		SwatchRecipe->SwatchCDO->mProducedIn.AddUnique(this->BuildGunBPClass);
	}

	// Modify instance properties of the generated class
	if (SwatchRecipe->SwatchInst)
	{
		SwatchRecipe->SwatchInst->mDisplayName = SwatchDescCDO->mDisplayName;
		SwatchRecipe->SwatchInst->HashName = genName;
		SwatchRecipe->SwatchInst->mCustomizationDesc = SwatchDescCDO->GetClass();
		SwatchRecipe->SwatchInst->mProducedIn.Empty();
		SwatchRecipe->SwatchInst->mProducedIn.AddUnique(this->BuildGunBPClass);

		this->SwatchRecipeArray.Add(UniqueID, SwatchRecipe->SwatchInst);
		return SwatchRecipe->SwatchInst;
	}
	
	return nullptr;
}

bool AUniversalSwatchSlotsSubsystem::GenerateNewSwatchUsingInfo(UUSSSwatchGroup* SwatchGroup, FUSSSwatch SwatchInfo)
{
	int32 slotID = this->ValidSlotIDs[0];

	// We can't overwrite existing swatch. (Well we could but I don't want to in order to not mess up evrything)
	if (this->SwatchDescriptorArray.Contains(slotID) || this->SwatchRecipeArray.Contains(slotID))
	{
		return false;
	}

	int32 nameCount = 0;
	int32* tmpPtr = this->SwatchNameCount.Find(SwatchInfo.Name.ToString());

	if (tmpPtr)
	{	// The name already exist in the list we need to generate a new one

		nameCount = *tmpPtr;
		*tmpPtr += 1;			// It is important to put this line after as the name count start at 1 if it already exist
	}
	else
	{	// The name doesn't exist yet

		this->SwatchNameCount.Add(SwatchInfo.Name.ToString(), 1);
	}

	FString genName = UUSSBPLib::BuildSwatchGenName(SwatchInfo.Name.ToString(), "SD", nameCount);

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

	this->ValidSlotIDs.RemoveAt(0);

	this->GenerateDynamicSwatchRecipe(slotID, this->GenerateDynamicSwatchDescriptor(slotID, genName, SwatchGroup, SwatchInfo));

	return true;
}


void AUniversalSwatchSlotsSubsystem::PatchBuildingsSwatchDescriptor()
{
	TArray<AActor*> foundActors;
	bool shouldPatch = false;

	if (this->SaveVersion != CurrVersion)
	{	// We need to make a check anyway

		shouldPatch = true;
		UE_LOG(LogUSS_Subsystem, Display, TEXT("Version mismatch, save version : %d, expected : %d. Patching buildings descriptor..."), this->SaveVersion, CurrVersion);
	}
	else
	{
		for (auto Match : this->InternalSwatchMatch)
		{
			if (Match.Key != Match.Value)
			{	// We need to update

				shouldPatch = true;
				UE_LOG(LogUSS_Subsystem, Display, TEXT("Palette mismatch for swatch ID %d at ID %d. Patching buildings descriptor..."), Match.Key, Match.Value);
				break;
			}
		}
	}

	if (shouldPatch)
	{	// We should patch buildings

		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildable::StaticClass(), foundActors);

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

					FUSSSwatchDescGenInfo* newDesc = this->USSInst->SwatchDescriptorArray.Find(*matchID);

					if (newDesc)
					{	// There is a valid swatch descriptor

						castBuild->mCustomizationData.ColorSlot = (uint8)*matchID;
						castBuild->mCustomizationData.SwatchDesc = newDesc->SwatchClass;
						castBuild->SetCustomizationData_Native(castBuild->mCustomizationData);
						UE_LOG(LogUSS_Subsystem, Warning, TEXT("Patching descriptor \"%s\" with \"%s\" named \"%s\" for building \"%s\"."), *castDesc->GetPathName(), *newDesc->SwatchClass->GetPathName(), *(newDesc->SwatchInst)->mDisplayName.ToString(), *castBuild->GetName());
					}
					else
					{	// There is no valid descriptor

						UE_LOG(LogUSS_Subsystem, Warning, TEXT("Can't patch \"%s\" for building \"%s\". No matching descriptor found"), *castDesc->GetPathName(), *castBuild->GetName());
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


void AUniversalSwatchSlotsSubsystem::ResetSubSystem()
{
	/*if (this->USSInst)
	{
		for (auto& tmp : this->USSInst->SwatchRecipeArray)
		{
			if (tmp.Value.SwatchInst->IsValidLowLevelFast())
			{
				tmp.Value.SwatchInst->ConditionalBeginDestroy();
				tmp.Value.SwatchInst = nullptr;
			}
		}

		for (auto& tmp : this->USSInst->SwatchGroupArray)
		{
			if (tmp.Value.SwatchInst->IsValidLowLevelFast())
			{
				tmp.Value.SwatchInst->ConditionalBeginDestroy();
				tmp.Value.SwatchInst = nullptr;
			}
		}

		for (auto& tmp : this->USSInst->SwatchDescriptorArray)
		{
			if (tmp.Value.SwatchInst->IsValidLowLevelFast())
			{
				tmp.Value.SwatchInst->ConditionalBeginDestroy();
				tmp.Value.SwatchInst = nullptr;
			}
		}
	}*/
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
	UE_LOG(LogUSS_Subsystem, Display, TEXT("Update save version to %d"), this->SaveVersion);
}
