// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"

#include "RenderUtils.h"
#include "Math/Vector2D.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Kismet/GameplayStatics.h"

#include "FGSchematic.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertyInteger.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Configuration/Properties/ConfigPropertyString.h"
#include "Configuration/Properties/ConfigPropertyArray.h"
#include "Configuration/Properties/ConfigPropertyBool.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlots, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlots)

void UUniversalSwatchSlotsWorldModule::RefreshColorSlots()
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	for (int32 i = 0; i < Subsystem->mColorSlots_Data.Num(); i++)
	{

		FFactoryCustomizationColorSlot ColorSlot = Subsystem->mColorSlots_Data[i];
		FGGameState->Server_SetBuildingColorDataForSlot(i, ColorSlot);

		FTimerDelegate TimerDel;
		FTimerHandle   TimerHandle;
		TimerDel.BindUFunction(Subsystem, FName("SetColorSlot_Data"), i, ColorSlot);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 2.0f, false);
		TimerDel.BindUFunction(FGGameState, FName("Server_SetBuildingColorDataForSlot"), i, ColorSlot);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 2.0f, false);

		FGGameState->Server_SetBuildingColorDataForSlot(i, ColorSlot);
	}
}


void UUniversalSwatchSlotsWorldModule::AddNewSwatchesColorSlots(TArray<FUSSSwatchInformation> SwatchInformations)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState) {
		for (FUSSSwatchInformation Swatch : SwatchInformations) {
			if (Swatch.mSwatch) {
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = Swatch.mSwatch.GetDefaultObject();
				if (SwatchDefauls) {
					uint8 ColourIndex = SwatchDefauls->ID;
					SwatchDefauls->mMenuPriority = static_cast<float>(ColourIndex);
					//if (!mSwatchIDMap.Find(ColourIndex)) {
						

						if (!Subsystem->mColorSlots_Data.IsValidIndex(ColourIndex)) {
							UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Try to add new color Slot at index, %d - %d"), ColourIndex, Subsystem->mColorSlots_Data.Num());
							for (uint8 i = Subsystem->mColorSlots_Data.Num(); i <= ColourIndex; ++i) {
								// Defaults
								FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(Swatch.mPrimaryColour, Swatch.mSecondaryColour);

								// Add to Array
								Subsystem->mColorSlots_Data.Add(NewColourSlot);
								FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
								FGGameState->Server_SetBuildingColorDataForSlot(i, NewColourSlot);

								FTimerDelegate TimerDel;
								FTimerHandle   TimerHandle;
								TimerDel.BindUFunction(Subsystem, FName("SetColorSlot_Data"), i, NewColourSlot);
								GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 2.0f, false);
								TimerDel.BindUFunction(FGGameState, FName("Server_SetBuildingColorDataForSlot"), i, NewColourSlot);
								GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 2.0f, false);

								Subsystem->SetColorSlot_Data(i, NewColourSlot);

								// Mark Slots as Dirty
								Subsystem->mColorSlotsAreDirty = true;

								UE_LOG(LogUniversalSwatchSlots, Log, TEXT("New Colour slot added: %d"), i);
							}
						}

						// Add to Array
						if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex)) {
							FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
							FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
							FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
							UE_LOG(LogUniversalSwatchSlots, Log, TEXT("write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
						}

						FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
						Subsystem->mColorSlotsAreDirty = true;
						UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Swatch found and success: %d > %s (%d/%d)"), ColourIndex, *Swatch.mSwatch->GetName(), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
					//}
					//else {
					//	UE_LOG(CustomizerSubsystem, Fatal, TEXT("Duplicate Swatch ID: %s | %d >< %s | %d"), *Swatch.mSwatch->GetName(), Swatch.mSwatch.GetDefaultObject()->ID, *mSwatchIDMap[ColourIndex]->GetName(), ColourIndex)
					//}

				}
			}
		}

		TArray<FFactoryCustomizationColorSlot> ColorSlots = Subsystem->mColorSlots_Data;
		UE_LOG(LogUniversalSwatchSlots, Error, TEXT("Slots: %d"), ColorSlots.Num());
		FGGameState->SetupColorSlots_Data(ColorSlots);
		//FGGameState->Init();
		Subsystem->mColorSlotsAreDirty = true;

		return;
	}

	if (FGGameState) {
		for (FUSSSwatchInformation Swatch : SwatchInformations) {
			if (Swatch.mSwatch) {
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = Swatch.mSwatch.GetDefaultObject();
				if (SwatchDefauls) {
					uint8 ColourIndex = SwatchDefauls->ID;
					SwatchDefauls->mMenuPriority = static_cast<float>(ColourIndex);
					if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex)) {
						FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
						FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
						FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
						UE_LOG(LogUniversalSwatchSlots, Log, TEXT("write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
					}
				}
			}
		}
	}

	UE_LOG(LogUniversalSwatchSlots, Error, TEXT("Cannot load AFGBuildableSubsystem for Swatches"));
	return;
}


// Thanks to Kyrium for the basic skeleton of this function !
/*void UUniversalSwatchSlotsWorldModule::AddNewSwatchesColorSlots(TArray<FUSSSwatchInformation> SwatchInformations)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState)
	{
		int32 StartSwatchID = Subsystem->mColorSlots_Data.Num();

		for (FUSSSwatchInformation& Swatch : SwatchInformations)
		{
			if (Swatch.mSwatch)
			{
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefaults = Swatch.mSwatch.GetDefaultObject();
				if (SwatchDefaults)
				{
					int32 ColourIndex = SwatchDefaults->ID;

					FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(FLinearColor::Black, FLinearColor::Black);
					NewColourSlot.PaintFinish = this->PaintFinish;

					if (Subsystem->mColorSlots_Data.Num() <= ColourIndex)
					{	// We need to create some default swatch slots

						for (int32 i = Subsystem->mColorSlots_Data.Num(); i <= ColourIndex; ++i)
						{
							if (!Subsystem->mColorSlots_Data.IsValidIndex(i))
							{	// We need to add a new slot
								Subsystem->mColorSlots_Data.Add(NewColourSlot);
								FGGameState->mBuildingColorSlots_Data.Add(NewColourSlot);
								UE_LOG(LogUniversalSwatchSlots, Log, TEXT("New Colour slot added to subsystem and gamestate: %d"), i);
							}
						}
					}

					// Changes the colour to the desired one
					NewColourSlot.PrimaryColor = Swatch.mPrimaryColour;
					NewColourSlot.SecondaryColor = Swatch.mSecondaryColour;

					// Update the subsystem and game state 
					Subsystem->SetColorSlot_Data(ColourIndex, NewColourSlot);
					FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);

					// Mark Slots as Dirty
					FGGameState->mBuildingColorSlots_Data[ColourIndex] = NewColourSlot;
					FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
					Subsystem->mColorSlotsAreDirty = true;


					FGGameState->Server_SetBuildingColorDataForSlot(ColourIndex, NewColourSlot);

					FTimerDelegate TimerDel;
					FTimerHandle   TimerHandle;
					TimerDel.BindUFunction(Subsystem, FName("SetColorSlot_Data"), ColourIndex, NewColourSlot);
					GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1.0f, false);
					TimerDel.BindUFunction(FGGameState, FName("Server_SetBuildingColorDataForSlot"), ColourIndex, NewColourSlot);
					GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1.0f, false);

					FGGameState->Server_SetBuildingColorDataForSlot(ColourIndex, NewColourSlot);

					UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Swatch found and success: %d > %s (%d/%d)"), ColourIndex, *Swatch.mSwatch->GetName(), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());

				}
			}
		}

		TArray<FFactoryCustomizationColorSlot> ColorSlots = Subsystem->mColorSlots_Data;
		UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Slots: %d"), ColorSlots.Num());
		FGGameState->SetupColorSlots_Data(ColorSlots);
		Subsystem->mColorSlotsAreDirty = true;

		return;
	}

	if (FGGameState) {
		for (FUSSSwatchInformation& Swatch : SwatchInformations) {
			if (Swatch.mSwatch) {
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = Swatch.mSwatch.GetDefaultObject();
				if (SwatchDefauls) {
					uint8 ColourIndex = SwatchDefauls->ID;
					SwatchDefauls->mMenuPriority = static_cast<float>(ColourIndex);
					if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex))
					{
						FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
					}
					FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
					FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
					UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
				}
			}
		}
	}

	UE_LOG(LogUniversalSwatchSlots, Error, TEXT("Cannot load AFGBuildableSubsystem for Swatches"));
	return;
}
*/

void UUniversalSwatchSlotsWorldModule::UpdateSwatchDescriptorIDCDO(UFGFactoryCustomizationDescriptor_Swatch* Target, int32 NewID)
{
	if (Target)
	{
		UFGFactoryCustomizationDescriptor_Swatch * CDO = Cast<UFGFactoryCustomizationDescriptor_Swatch>(Target->GetClass()->GetDefaultObject());

		Target->ID = NewID;

		if (CDO)
		{
			CDO->ID = NewID;
		}
	}
}


UFGCustomizerSubCategory* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority)
{
	if (this->SwatchGroupArray.Contains(UniqueGroupID))
	{	// The group already exist

		UFGCustomizerSubCategory* SGCDO = Cast<UFGCustomizerSubCategory>((*this->SwatchGroupArray.Find(UniqueGroupID))->GetClass()->GetDefaultObject());
		if (SGCDO)
		{	// Overwrite the group name and priority

			SGCDO->mDisplayName = DisplayName;
			SGCDO->mMenuPriority = Priority;
		}

		return *this->SwatchGroupArray.Find(UniqueGroupID);
	}
	else
	{
		// Create a dynamic derivated class
		//UClass* NewClass = GenerateDynamicClass(UFGCustomizerSubCategory::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID)));
		UClass* NewClass = GenerateDynamicClass(UUSSGroup::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID)));

		if (NewClass)
		{
			UObject* tempCDO = NewClass->GetDefaultObject();

			if (tempCDO)
			{
				// Modify CDO properties of the newly generated class
				UFGCustomizerSubCategory* CDO = Cast<UFGCustomizerSubCategory>(tempCDO);

				if (CDO)
				{
					CDO->mDisplayName = DisplayName;
					CDO->mMenuPriority = Priority;
				}

				// Create an instance to return
				UFGCustomizerSubCategory* InstClass = NewObject<UFGCustomizerSubCategory>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_Gen_USS_SwatchGroup_%d"), UniqueGroupID)), RF_MarkAsRootSet | RF_Public);

				if (InstClass)
				{	// For unknown reason this instance is not initialized using our modified CDO...

					InstClass->mDisplayName = DisplayName;
					InstClass->mMenuPriority = Priority;

					this->SwatchGroupArray.Add(UniqueGroupID, InstClass);
				}

				this->GeneratedClasses.Add(NewClass);
				return InstClass;
			}
		}
	}
	
	return nullptr;
}


UFGFactoryCustomizationDescriptor_Swatch* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchDescriptor(int32 SlotID, int32 SwapID, FText DisplayName, FString GenName, float Priority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory* SwatchGroup, bool& HasSwapped)
{
	// Create a dynamic derivated class
	//UClass* NewClass = GenerateDynamicClass(UFGFactoryCustomizationDescriptor_Swatch::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), UniqueID)));
	UClass* NewClass = GenerateDynamicClass(UFGFactoryCustomizationDescriptor_Swatch::StaticClass(), FName(*GenName));

	if (NewClass)
	{
		UObject* tCDO = NewClass->GetDefaultObject();
		
		if (tCDO)
		{
			// Modify CDO properties of the newly generated class
			UFGFactoryCustomizationDescriptor_Swatch* CDO = Cast<UFGFactoryCustomizationDescriptor_Swatch>(tCDO);

			if (CDO)
			{
				CDO->ID = SlotID;
				CDO->mDisplayName = DisplayName;
				CDO->mDescription = this->SwatchDescription;
				CDO->mIcon = this->GenerateSwatchIcon(PrimaryColor, SecondaryColor);
				CDO->mPersistentBigIcon = CDO->mIcon.Get();
				CDO->mSmallIcon = CDO->mIcon.Get();
				CDO->mCategory = this->SwatchCategory;
				CDO->mMenuPriority = Priority;
				if (SwatchGroup != nullptr) CDO->mSubCategories.Add(SwatchGroup->GetClass());
			}

			// Create an instance to return
			UFGFactoryCustomizationDescriptor_Swatch* InstClass = NewObject<UFGFactoryCustomizationDescriptor_Swatch>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *GenName)), RF_MarkAsRootSet | RF_Public);
			
			if (InstClass)
			{	// For unknown reason this instance is not initialized using our modified CDO...
			
				InstClass->ID = SlotID;
				InstClass->mDisplayName = DisplayName;
				InstClass->mDescription = this->SwatchDescription;
				InstClass->mIcon = CDO->mIcon;
				InstClass->mPersistentBigIcon = CDO->mPersistentBigIcon;
				InstClass->mSmallIcon = CDO->mSmallIcon;
				InstClass->mCategory = this->SwatchCategory;
				InstClass->mMenuPriority = Priority;
				if (SwatchGroup != nullptr) InstClass->mSubCategories.Add(SwatchGroup->GetClass());
			}

			//if (!this->SwatchDescriptorArray.Contains(UniqueID))
			UFGFactoryCustomizationDescriptor_Swatch** tmpSD = this->SwatchDescriptorArray.Find(SlotID);
			if (tmpSD == nullptr)
			{	// We can safely add this swatch descriptor

				this->SwatchDescriptorArray.Add(SlotID, InstClass);
			}
			else
			{	// If we are here it means that a previous swatch had no corresponding saved swatch and was created before this one. We need to update its CDO in order to keep the correct swatch slot IDs.

				UpdateSwatchDescriptorIDCDO(*tmpSD, SwapID);		// Update the previous swatch CDO ID

				this->SwatchDescriptorArray.Add(SwapID, *tmpSD);	// Move the previous swatch to its new slot
				*tmpSD = InstClass;									// Add the new swatch to its correct slot
				HasSwapped = true;

				// We must update the recipe of the swapped swatch otherwise it will be overwritten
				UFGCustomizationRecipe* tmpSR = nullptr;
				this->SwatchRecipeArray.RemoveAndCopyValue(SlotID, tmpSR);
				this->SwatchRecipeArray.Add(SwapID, tmpSR);

				UE_LOG(LogUniversalSwatchSlots, Verbose, TEXT("Swaping slot color IDs: %d <-> %d"), SlotID, SwapID);
			}

			this->GeneratedClasses.Add(NewClass);

			return InstClass;
		}
	}

	return nullptr;
}


UFGCustomizationRecipe* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchRecipe(UFGFactoryCustomizationDescriptor_Swatch* SwatchDescriptor)
{
	if (SwatchDescriptor == nullptr)
	{
		return nullptr;
	}

	UFGFactoryCustomizationDescriptor_Swatch* SwatchDescCDO = (UFGFactoryCustomizationDescriptor_Swatch *) SwatchDescriptor->GetClass()->GetDefaultObject();
	
	// Create a dynamic derivated class
	FString GenName = AUniversalSwatchSlotsSubsystem::BuildSwatchGenName(SwatchDescCDO->mDisplayName.ToString(), "SR", SwatchDescCDO->ID);
	UClass* NewClass = GenerateDynamicClass(UFGCustomizationRecipe::StaticClass(), FName(*GenName));

	if (NewClass)
	{
		UObject* tempCDO = NewClass->GetDefaultObject();

		if (tempCDO)
		{
			// Modify CDO properties of the newly generated class
			UFGCustomizationRecipe* CDO = Cast<UFGCustomizationRecipe>(tempCDO);
			if (CDO)
			{
				CDO->mDisplayName = SwatchDescCDO->mDisplayName;
				CDO->mCustomizationDesc = SwatchDescCDO->GetClass();
				CDO->mProducedIn.Add(this->BuildGunBPClass);
			}

			// Create an instance to return
			UFGCustomizationRecipe* InstClass = NewObject<UFGCustomizationRecipe>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *GenName)), RF_MarkAsRootSet | RF_Public);

			if (InstClass)
			{	// For unknown reason this instance is not initialized using our modified CDO...

				InstClass->mDisplayName = SwatchDescCDO->mDisplayName;
				InstClass->mCustomizationDesc = SwatchDescCDO->GetClass();
				InstClass->mProducedIn.Add(this->BuildGunBPClass);

				if (!this->SwatchRecipeArray.Contains(SwatchDescCDO->ID))
				{
					this->SwatchRecipeArray.Add(SwatchDescCDO->ID, InstClass);
				}
			}

			this->GeneratedClasses.Add(NewClass);
			return InstClass;
		}
	}

	return nullptr;
}


bool UUniversalSwatchSlotsWorldModule::GenerateNewSwatch(int32 UniqueGroupID, FText GroupDisplayName, float GroupPriority, int32 SwatchUniqueID, FText SwatchDisplayName, float SwatchPriority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe)
{
	int32 slotID = this->ValidSlotIDs[0];


	if (this->SwatchDescriptorArray.Contains(slotID) || this->SwatchRecipeArray.Contains(slotID))
	{	// We can't overwrite existing swatch. (Well we could but I don't want to in order to not mess up evrything)

		SwatchGroup = nullptr;
		SwatchDescriptor = nullptr;
		SwatchRecipe = nullptr;
		return false;
	}

	SwatchGroup = this->GenerateDynamicSwatchGroup(UniqueGroupID, GroupDisplayName, GroupPriority);

	int32 nameCount = 0;
	int32* tmpPtr = this->SwatchNameCount.Find(SwatchDisplayName.ToString());
	
	if (tmpPtr)
	{	// The name already exist in the listwe need to generate a new one

		nameCount = *tmpPtr;
		*tmpPtr += 1;			// It is important to put this line after as the name count start at 1 if it already exist
	}
	else
	{	// The name doesn't exist yet

		this->SwatchNameCount.Add(SwatchDisplayName.ToString(), 1);
	}

	FString genName = AUniversalSwatchSlotsSubsystem::BuildSwatchGenName(SwatchDisplayName.ToString(), "SD", nameCount);

	FUSSSwatchSaveInfo tmpSaved;
	bool hasSwapped = false;

	// Try to find if there is a saved swatch matching this swatch
	if (this->USSSubsystem->FindSavedSwatch(genName, tmpSaved))
	{	// There is one

		slotID = tmpSaved.SwatchSlotID;
		UE_LOG(LogUniversalSwatchSlots, Verbose, TEXT("Found saved color \"%s\" at slot : %d"), *genName, slotID); 
		SwatchDescriptor = this->GenerateDynamicSwatchDescriptor(slotID, this->ValidSlotIDs[0], SwatchDisplayName, genName, SwatchPriority, PrimaryColor, SecondaryColor, SwatchGroup, hasSwapped);

		if (hasSwapped)
		{	// A swap occured

			this->ValidSlotIDs.RemoveAt(0);
		}
		else
		{	// Remove the one corresponding to the slot ID

			this->ValidSlotIDs.Remove(slotID);
		}
	}
	else
	{	// We can remove this slot from the valid one.

		SwatchDescriptor = this->GenerateDynamicSwatchDescriptor(slotID, SwatchUniqueID, SwatchDisplayName, genName, SwatchPriority, PrimaryColor, SecondaryColor, SwatchGroup, hasSwapped);
		this->ValidSlotIDs.RemoveAt(0);
	}


	SwatchRecipe = this->GenerateDynamicSwatchRecipe(SwatchDescriptor);

	return true;
}


bool UUniversalSwatchSlotsWorldModule::GenerateNewSwatchUsingInfo(FUSSSwatch SwatchInformation, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe)
{
	return this->GenerateNewSwatch(SwatchInformation.UniqueGroupID, SwatchInformation.GroupDisplayName, SwatchInformation.GroupPriority, SwatchInformation.SwatchUniqueID, SwatchInformation.SwatchDisplayName, SwatchInformation.SwatchPriority, SwatchInformation.PrimaryColour, SwatchInformation.SecondaryColour, SwatchGroup, SwatchDescriptor, SwatchRecipe);
}


/*UClass* UUniversalSwatchSlotsWorldModule::GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName)
{
	if (!TemplateClass || !TemplateClass->IsValidLowLevelFast())
	{
		return nullptr;
	}

	UClass* NewClass = NewObject<UClass>(this->PersistentPackage, GeneratedClassName, RF_Public | RF_Standalone);

	if (NewClass)
	{
		// Setting UCLASS properties
		NewClass->PurgeClass(false);
		NewClass->ClassFlags |= CLASS_Optional;
		NewClass->PropertyLink = TemplateClass->PropertyLink;

		// Setting class constructor
		NewClass->ClassConstructor = TemplateClass->ClassConstructor;
		NewClass->ClassWithin = TemplateClass->ClassWithin;

		// Setting parent class
		NewClass->SetSuperStruct(TemplateClass);

		// Compile class
		NewClass->StaticLink(true);
		NewClass->Bind();

		PersistentPackage->SetDirtyFlag(true); // Indiquer que le package a été modifié
	}

	return NewClass;
}*/


UClass* UUniversalSwatchSlotsWorldModule::GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName)
{
	if (!TemplateClass || !TemplateClass->IsValidLowLevelFast())
	{
		return nullptr;
	}

	UClass* NewClass = NewObject<UClass>(GetTransientPackage(), GeneratedClassName, RF_Public | RF_Transient);

	if (NewClass)
	{
		// Setting UCLASS properties
		NewClass->PurgeClass(false);
		NewClass->ClassFlags |= CLASS_Transient;
		NewClass->PropertyLink = TemplateClass->PropertyLink;

		// Setting class constructor
		NewClass->ClassConstructor = TemplateClass->ClassConstructor;
		NewClass->ClassWithin = TemplateClass->ClassWithin;

		// Setting parent class
		NewClass->SetSuperStruct(TemplateClass);

		// Compile class
		NewClass->StaticLink(true);
		NewClass->Bind();
	}

	return NewClass;
}


bool UUniversalSwatchSlotsWorldModule::ParseModConfig()
{
	if (this->RootConfig == nullptr)
	{
		UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("The /FactoryGame/Configs/UniversalSwatchSlots.cfg does not exist or permission denied. No swatch can be created."));
		return false;
	}

	UConfigPropertySection* RootSec = (UConfigPropertySection*)this->RootConfig;

	if (RootSec)
	{	// The configuration has a root section

		UConfigPropertyArray* Associations = (UConfigPropertyArray*) *RootSec->SectionProperties.Find("Associations");
		UConfigPropertyArray* PalettesArr = (UConfigPropertyArray*) *RootSec->SectionProperties.Find("Palettes");

		if (Associations == nullptr)
		{	// It is useless to check for pelettes if we don't need to apply them to at least one session

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Associations' section found in the config file. No swatch can be created."));
			return false;
		}

		if (PalettesArr == nullptr)
		{	// There is no palette found
			
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Palettes' section found in the config file. No swatch can be created."));
			return false;
		}
		
		this->ParseAssociations(Associations);
		this->ParsePalettes(PalettesArr);
	}
	else
	{
		UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No root found in the configuration file. No swatch can be created."));
		return false;
	}

	return true;
}


void UUniversalSwatchSlotsWorldModule::ParseAssociations(UConfigPropertyArray* Associations)
{
	int32 assoID = 0;

	for (UConfigProperty* AssociationProp : Associations->Values)
	{	// Check all palette / session associations

		UConfigPropertySection* association = (UConfigPropertySection*)AssociationProp;

		if (association == nullptr)
		{	// Invalid association

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Invalid session / palette association found in section 'Associations' at index %d"), assoID);
		}

		if (association->SectionProperties.Contains("PaletteName"))
		{	// There is a palette name given to this association

			if (association->SectionProperties.Contains("Sessions"))
			{	// There is some sessions to apply the palette to

				FUSSPalette newPalette;

				newPalette.PaletteName = ((UConfigPropertyString*)*association->SectionProperties.Find("PaletteName"))->Value;

				if (!this->Palettes.Contains(newPalette.PaletteName))
				{	// The association doesn't exit

					UConfigPropertyArray* SessionArray = (UConfigPropertyArray*)*association->SectionProperties.Find("Sessions");
					
					int32 sessionID = 0;

					for (UConfigProperty* SessionProp : SessionArray->Values)
					{	// Check all sessions

						if (SessionProp)
						{
							UConfigPropertySection* session = (UConfigPropertySection*)SessionProp;

							FUSSSession newSession;

							if (session->SectionProperties.Contains("Name"))
							{	// Add the session to the array

								newSession.SessionName = ((UConfigPropertyString*)*session->SectionProperties.Find("Name"))->Value;

								if (session->SectionProperties.Contains("AddPrim"))
								{
									newSession.AddPrimaryColorsToPreset = ((UConfigPropertyBool*)*session->SectionProperties.Find("AddPrim"))->Value;
								}
								else
								{	// Put it to false

									newSession.AddPrimaryColorsToPreset = false;
									UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'AddPrim' key found in 'Sessions' number % in 'Associations' at index %d. The default value will be used : false."), sessionID, assoID);
								}

								if (session->SectionProperties.Contains("AddSec"))
								{
									newSession.AddSecondaryColorsToPreset = ((UConfigPropertyBool*)*session->SectionProperties.Find("AddSec"))->Value;
								}
								else
								{	// Put it to false

									newSession.AddSecondaryColorsToPreset = false;
									UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'AddPrim' key found in 'Sessions' number % in 'Associations' at index %d. The default value will be used : false."), sessionID, assoID);
								}

								newPalette.AssociatedSessions.Add(newSession);
							}
							else
							{	// Ignore this session

								UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'Sessions' number % in 'Associations' at index %d. This association will be ignored."), sessionID, assoID);
							}

							sessionID++;
						}
					}

					this->Palettes.Add(newPalette.PaletteName, newPalette);
				}
				else
				{	// Ignore this association

					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("The 'Associations' at index %d already exist. Only the first one will be kept."), assoID);
				}
			}
			else
			{	// Ignore this association

				UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Sessions' key found in 'Associations' at index %d. This association will be ignored."), assoID);
			}
		}
		else
		{	// Ignore this association

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'PaletteName' key found in 'Associations' at index %d. This association will be ignored."), assoID);
		}

		assoID++;
	}

}


void UUniversalSwatchSlotsWorldModule::ParsePalettes(UConfigPropertyArray* PalettesArr)
{
	int32 palID = 0;

	for (UConfigProperty* PaletteProp : PalettesArr->Values)
	{	// Check all palettes

		UConfigPropertySection* palette = (UConfigPropertySection*)PaletteProp;

		if (palette == nullptr)
		{	// Invalid palette

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Invalid palette found in section 'Palettes' at index %d"), palID);
		}

		if (palette->SectionProperties.Contains("PaletteName"))
		{	// There is a palette name given to this association

			FString palName = ((UConfigPropertyString*)*palette->SectionProperties.Find("PaletteName"))->Value;

			if (this->Palettes.Contains(palName))
			{	// There is at least one assocation for this palette

				FUSSPalette* newPalette = this->Palettes.Find(palName);

				for (FUSSSession AssociatedSession : newPalette->AssociatedSessions)
				{	// Find if the desired session is in this palette session association

					if (AssociatedSession.SessionName.Equals(this->SessionName))
					{	// Parse the palette as we need to apply it to the session

						UE_LOG(LogUniversalSwatchSlots, Display, TEXT("Find matching session \"%s\" for palette \"%s\". Loading palette."), *AssociatedSession.SessionName, *palName, *this->SessionName);

						if (palette->SectionProperties.Contains("SwatchGroups"))
						{	// SwatchGroups key found

							UConfigPropertyArray* SwatchGroups = (UConfigPropertyArray*)*palette->SectionProperties.Find("SwatchGroups");

							int32 groupID = 0;
							int32 startValidSlotID = 0;

							for (UConfigProperty* GroupProp : SwatchGroups->Values)
							{	// Check all swatch groups

								startValidSlotID = this->ParseSwatchGroup(startValidSlotID, groupID, (UConfigPropertySection*)GroupProp, newPalette);
								groupID++;
							}
						}
						else
						{	// No swatch group. Ignore this palette.

							UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'SwatchGroups' section found for palette \"%s\" at index %d. This palette will be ignored."), *palName, palID);
						}
						return;
					}
					else
					{	// Ignore the palette
						
						UE_LOG(LogUniversalSwatchSlots, Display, TEXT("Find session \"%s\" for palette \"%s\" but does not match desired session \"%s\". Ignoring palette for now."), *AssociatedSession.SessionName, *palName, *this->SessionName);
					}
				}
			}
			else
			{	// Ignore this palette
				
				UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No association found for palette %s at index %d. This palette will be ignored."), *palName, palID);
			}
		}
		else
		{	// Ignore this palette

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'PaletteName' key found in 'Palette' at index %d. This palette will be ignored."), palID);
		}

		palID++;
	}
}


int32 UUniversalSwatchSlotsWorldModule::ParseSwatchGroup(int32 StartValidSlotID, int32 GroupID, UConfigPropertySection* SwatchGroup, FUSSPalette* OutPalette)
{
	int32 finalValidSlotID = StartValidSlotID;

	if (SwatchGroup)
	{	// The swatch group is valid

		FString groupName = FString("Group ") + FString::FromInt(GroupID);
		float groupPriority = 0.0;

		if (SwatchGroup->SectionProperties.Contains("Name"))
		{	// A name has been given

			groupName = ((UConfigPropertyString*)*SwatchGroup->SectionProperties.Find("Name"))->Value;
		}
		else
		{	// No name given, build default

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'SwatchGroup' number %d in palette %s. Default will be used : %s."), GroupID, *OutPalette->PaletteName, *groupName);
		}

		if (SwatchGroup->SectionProperties.Contains("Priority"))
		{	// A priority has been given

			groupPriority = ((UConfigPropertyFloat*)*SwatchGroup->SectionProperties.Find("Priority"))->Value;
		}
		else
		{	// No priority given, use default

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'SwatchGroup' number %d in palette %s. Default will be used : %s."), GroupID, *OutPalette->PaletteName, *groupName);
		}

		if (SwatchGroup->SectionProperties.Contains("Swatches"))
		{	// The group has some swatches
			
			UConfigPropertyArray* Swatches = (UConfigPropertyArray*)*SwatchGroup->SectionProperties.Find("Swatches");

			for (UConfigProperty* SwatchProp : Swatches->Values)
			{	// Check all swatches

				if (this->ValidSlotIDs.Num() < 1)
				{
					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Can't have more than 255 swatch slots. Ignoring the rest."));
					return 255;
				}

				int32 swatchID = this->ValidSlotIDs[finalValidSlotID];
				if (swatchID >= 255)
				{	// ID 255 is used for the custom swatch

					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Can't have more than 255 swatch slots. Ignoring the rest."));
					return 255;
				}

				if (!this->ParseSwatch(swatchID, (UConfigPropertySection*)SwatchProp, GroupID, groupName, groupPriority, OutPalette))
				{	// A swatch has been parsed. We can increment the swatch ID

					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("There was a problem parsing swatch %d.Ignoring the rest."), swatchID);
				}
				else
				{
					finalValidSlotID++;
				}
			}

			return finalValidSlotID;
		}
		else
		{	// The group has no swatches, we could ignore it 
		
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Swatches' key found in 'SwatchGroup' %s in palette %s. The group will be ignored."), *groupName, *OutPalette->PaletteName);
		}
	}

	return finalValidSlotID;
}


bool UUniversalSwatchSlotsWorldModule::ParseSwatch(int32 SwatchID, UConfigPropertySection* Swatch, int32 GroupID, FString GroupName, float GroupPriority, FUSSPalette* OutPalette)
{
	if (Swatch)
	{	// The swatch is valid
	
		FUSSSwatch newSwatch;
		newSwatch.UniqueGroupID = GroupID;
		newSwatch.GroupDisplayName = FText::FromString(GroupName);
		newSwatch.GroupPriority = GroupPriority;
		newSwatch.SwatchUniqueID = SwatchID;

		FString swatchName = FString("Swatch ") + FString::FromInt(SwatchID);
		FString defaultColor = FString("#FFFFFFFF");

		if (Swatch->SectionProperties.Contains("Name"))
		{	// A name has been given

			swatchName = ((UConfigPropertyString*)*Swatch->SectionProperties.Find("Name"))->Value;
		}
		else
		{	// No name given, build default

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'Swatch' number %d in palette %s in group %s. Default will be used : %s."), SwatchID, *OutPalette->PaletteName, *GroupName, *swatchName);
		}

		newSwatch.SwatchDisplayName = FText::FromString(swatchName);

		if (Swatch->SectionProperties.Contains("Priority"))
		{	// A name has been given

			newSwatch.SwatchPriority = ((UConfigPropertyFloat*)*Swatch->SectionProperties.Find("Priority"))->Value;
		}
		else
		{	// No name given, build default

			newSwatch.SwatchPriority = 0.0f;
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Priority' key found in 'Swatch' number %d in palette %s in group %s. Default will be used : %f."), SwatchID, *OutPalette->PaletteName, *GroupName, 0.0f);
		}

		if (Swatch->SectionProperties.Contains("Primary"))
		{	// A primary color has been given

			newSwatch.PrimaryColour = this->HexToLinearColor(((UConfigPropertyString*)*Swatch->SectionProperties.Find("Primary"))->Value);
		}
		else
		{	// No primary color given, use default

			newSwatch.PrimaryColour = this->HexToLinearColor(defaultColor);
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Primary' key found in 'Swatch' %s in palette %s in group %s. Default will be used : %s."), *swatchName, *OutPalette->PaletteName, *GroupName, *defaultColor);
		}

		if (Swatch->SectionProperties.Contains("Secondary"))
		{	// A secondary color has been given

			newSwatch.SecondaryColour = this->HexToLinearColor(((UConfigPropertyString*)*Swatch->SectionProperties.Find("Secondary"))->Value);
		}
		else
		{	// No secondary color given, use default
			
			newSwatch.SecondaryColour = this->HexToLinearColor(defaultColor);
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Secondary' key found in 'Swatch' %s in palette %s in group %s. Default will be used : %s."), *swatchName, *OutPalette->PaletteName, *GroupName, *defaultColor);
		}

		OutPalette->Swatches.Add(newSwatch);

		return true;
	}

	return false;
}


FLinearColor UUniversalSwatchSlotsWorldModule::HexToLinearColor(FString HexCode)
{
	if (HexCode.StartsWith("#"))
	{
		HexCode.RemoveAt(0); // Remove the '#' character
	}

	// Check for string validity
	if (HexCode.IsEmpty() || (HexCode.Len() != 6 && HexCode.Len() != 8))
	{
		UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Invalid Hex Color Format: %s"), *HexCode);
		return FLinearColor::Black;
	}

	return FLinearColor::FromSRGBColor(FColor::FromHex(HexCode));
}

UTexture2D* UUniversalSwatchSlotsWorldModule::GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor)
{
	int32 Width = 128;
	int32 ArcThickness = 15;
	FColor ArcColor = FColor(192, 192, 192, 255);
	FColor prim = PrimaryColor.ToFColor(true);
	FColor sec = SecondaryColor.ToFColor(true);

	if (Width <= 0 || ArcThickness <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid texture dimensions or arc thickness."));
		return nullptr;
	}

	// Check that the width is a power a 2 to ensure optimal compatibility
	if ((Width & (Width - 1)) != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Width is not a power of 2. The texture might not behave optimally in some scenarios."));
	}

	// Circle radius and center
	int32 Radius = Width / 2;
	FVector2D Center(Radius, Radius);

	// Create a transient texture
	UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Width, PF_B8G8R8A8);
	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create transient texture."));
		return nullptr;
	}

	// Prevent garbage collector to delete the texture
	NewTexture->AddToRoot();

	// Initialiser texture data
	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* Pixels = static_cast<FColor*>(Data);

	// Fill the texture
	for (int32 Y = 0; Y < Width; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			FVector2D Pixel(X, Y);
			float DistanceToCenter = FVector2D::Distance(Pixel, Center);

			if (DistanceToCenter > Radius - ArcThickness && DistanceToCenter <= Radius)
			{
				// Pixel dans l'arceau
				Pixels[Y * Width + X] = ArcColor;
			}
			else if (DistanceToCenter <= Radius - ArcThickness)
			{
				// Pixel inside the circle
				if (X >= Width - Y - 1) // Inverted diagonale 
				{
					Pixels[Y * Width + X] = sec;
				}
				else
				{
					Pixels[Y * Width + X] = prim;
				}
			}
			else
			{
				// Pixel en dehors du cercle
				Pixels[Y * Width + X] = FColor::Transparent; // Transparent color
			}
		}
	}

	// Unlock data and update texture
	Mip.BulkData.Unlock();
	NewTexture->UpdateResource();

	return NewTexture;
}


void UUniversalSwatchSlotsWorldModule::RetrieveFreeColorSlotID()
{
	int32 SFSlots = 28; // 0 - 17 swatches, 18 = concrete, 19 = carbon, 20 = caterium, 21 = chrome, 22 = copper, 23 = unpainted, 24 - 27 = project assembly
	
	int32 StartID = SFSlots;

	if (this->IsUsingMoreSwatchSlots)
	{	// At the moment MMS adds 20 pre-defined index for swatches (28 - 47)

		StartID += 20;
	}
	
	// 255 = custom
	for (int32 i = StartID; i < 255; i++)
	{
		this->ValidSlotIDs.Add(i);
	}
}


int32 UUniversalSwatchSlotsWorldModule::FindUSSStartSlotID()
{
	/*AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState)
	{
		return Subsystem->mColorSlots_Data.Num();	// SF swatch slots ends at index 28
	}
	return this->SwatchCollection->mCustomizations.Num();
	int32 startSlotID = 28; // SF slots end at slot 28
	
	if (this->SwatchCollection)
	{
		for (TSubclassOf <UFGFactoryCustomizationDescriptor> SwatchDesc : this->SwatchCollection->mCustomizations)
		{
			if (SwatchDesc.GetDefaultObject()->GetName().Contains("Gen_USS_"))
			{
				break;
			}
			startSlotID++;
		}
	}
	
	return startSlotID;*/
	return 0;
}


void UUniversalSwatchSlotsWorldModule::InitUSSGameWorldModule(bool CleanUpBeforeInit)
{
	this->PersistentPackage = CreatePackage(TEXT("/Game/DynamicClasses"));
	if (!PersistentPackage)
	{
		UE_LOG(LogUniversalSwatchSlots, Error, TEXT("Failed to create persistent package."));
	}

	// Définir des flags pour lier un fichier config
	//PersistentPackage->SetPackageFlags(PKG_Config);
	PersistentPackage->SetPackageFlags(PKG_RuntimeGenerated); // Si nécessaire
	PersistentPackage->MarkPackageDirty(); // Assurez-vous que le package est reconnu comme modifié

	if (CleanUpBeforeInit)
	{
		AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
		AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

		if (Subsystem && FGGameState)
		{
			int32 i = 28, j = i;
			if (this->IsUsingMoreSwatchSlots)
			{
				i = j = 48;
			}

			if (Subsystem->mColorSlots_Data.IsValidIndex(255))
			{	// Keep the custom color

				this->CustomColor = Subsystem->mColorSlots_Data[255];
			}

			for (i; i < Subsystem->mColorSlots_Data.Num() - 1; i++, j++)
			{
				if (Subsystem->mColorSlots_Data.IsValidIndex(i))
				{	// We need to add a new slot
					Subsystem->mColorSlots_Data.RemoveAt(i);
					FGGameState->mBuildingColorSlots_Data.RemoveAt(i);
					UE_LOG(LogUniversalSwatchSlots, Log, TEXT("Cleared Colour slot from subsystem: %d"), j);
					i--;
				}
			}
			FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
			Subsystem->mColorSlotsAreDirty = true;
		}

	}

	this->RetrieveFreeColorSlotID();
}

void UUniversalSwatchSlotsWorldModule::CleanUpPreviousSession()
{
	if (this->SwatchCollection)
	{
		this->SwatchCollection->mCustomizations.Empty();

		// Modify CDO
		UFGFactoryCustomizationCollection* CDO = (UFGFactoryCustomizationCollection*)this->SwatchCollection->GetClass()->GetDefaultObject();
		CDO->mCustomizations.Empty();

		this->SwatchCollection = nullptr;
	}

	if (this->SwatchDescriptorArray.Num() > 0)
	{	// Clean the color slots

		AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
		AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

		if (Subsystem && FGGameState)
		{
			int32 NumOfRemove = 0;
			TArray<int32> Keys;
			this->SwatchDescriptorArray.GetKeys(Keys);

			for (int32 SlotID : Keys)
			{	// Remove all slots used by the previous session

				if (Subsystem->mColorSlots_Data.IsValidIndex(SlotID - NumOfRemove))
				{
					Subsystem->mColorSlots_Data.RemoveAt(SlotID - NumOfRemove);
					NumOfRemove++;
				}
			}

			FGGameState->mBuildingColorSlots_Data.SetNum(Subsystem->mColorSlots_Data.Num(), false);
			FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
		}
	}

	this->SwatchGroupArray.Empty();
	this->SwatchDescriptorArray.Empty();
	this->SwatchRecipeArray.Empty();
	this->ValidSlotIDs.Empty();

	this->USSSubsystem = nullptr;
}