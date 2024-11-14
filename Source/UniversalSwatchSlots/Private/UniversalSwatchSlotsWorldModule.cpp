// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "FGSchematic.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Kismet/GameplayStatics.h"

DECLARE_LOG_CATEGORY_EXTERN(USSWorldModule, Log, All)

DEFINE_LOG_CATEGORY(USSWorldModule)


void UUniversalSwatchSlotsWorldModule::AddNewSwatchesColorSlots(TArray<FUSSSwatchInformation> SwatchInformations)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState) {
		for (FUSSSwatchInformation& Swatch : SwatchInformations)
		{
			if (Swatch.mSwatch)
			{
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = Swatch.mSwatch.GetDefaultObject();
				if (SwatchDefauls)
				{
					uint8 ColourIndex = SwatchDefauls->ID;
					SwatchDefauls->mMenuPriority = static_cast<float>(ColourIndex);
					//if (!mSwatchIDMap.Find(ColourIndex)) {
						/*if (mDefaultSwatchCollection) {
							UFGFactoryCustomizationCollection* Default = CDOHelperSubsystem->GetAndStoreDefaultObject_Native<UFGFactoryCustomizationCollection>(mDefaultSwatchCollection);
							if (IsValid(Default)) {
								Default->mCustomizations.AddUnique(Swatch.mSwatch);
							}
						}*/

						if (!Subsystem->mColorSlots_Data.IsValidIndex(ColourIndex))
						{
							UE_LOG(USSWorldModule, Log, TEXT("Try to add new color Slot at index, %d - %d"), ColourIndex, Subsystem->mColorSlots_Data.Num());
							
							for (uint8 i = Subsystem->mColorSlots_Data.Num(); i <= ColourIndex; ++i)
							{
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

								UE_LOG(USSWorldModule, Log, TEXT("New Colour slot added: %d"), i);
							}
						}

						// Add to Array
						if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex))
						{
							FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
							FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
							FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
							UE_LOG(USSWorldModule, Log, TEXT("write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
						}

						//mSwatchIDMap.Add(ColourIndex, Swatch.mSwatch);
						FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
						Subsystem->mColorSlotsAreDirty = true;
						UE_LOG(USSWorldModule, Log, TEXT("Swatch found and success: %d > %s (%d/%d)"), ColourIndex, *Swatch.mSwatch->GetName(), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
					//}
					//else {
					//	UE_LOG(CustomizerSubsystem, Fatal, TEXT("Duplicate Swatch ID: %s | %d >< %s | %d"), *Swatch.mSwatch->GetName(), Swatch.mSwatch.GetDefaultObject()->ID, *mSwatchIDMap[ColourIndex]->GetName(), ColourIndex)
					//}

					// Ignore Slots used by CSS (Slot 16 for example is used twice)
					/*if (!(ColourIndex > 18 && ColourIndex < 255)) {
						UE_LOG(CustomizerSubsystem, Fatal, TEXT("Please use a Index between 19 and 254 (Dont use Slots from SF!)"));
					}*/
				}
			}
		}

		TArray<FFactoryCustomizationColorSlot> ColorSlots = Subsystem->mColorSlots_Data;
		UE_LOG(USSWorldModule, Log, TEXT("Slots: %d"), ColorSlots.Num());
		FGGameState->SetupColorSlots_Data(ColorSlots);
		//FGGameState->Init();
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
					if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex)) {
						FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
						FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
						FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
						UE_LOG(USSWorldModule, Log, TEXT("write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
					}
				}
			}
		}
	}

	UE_LOG(USSWorldModule, Error, TEXT("Cannot load AFGBuildableSubsystem for Swatches"));
	return;
}


UFGCustomizerSubCategory* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchGroup(int32 UniqueGroupID, FText DisplayName, float Priority)
{
	//UFGCustomizerSubCategory* NewClass = nullptr;
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
		//NewClass = NewObject<UFGCustomizerSubCategory>(GetTransientPackage(), FName(*FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID)));
		
		UClass* NewClass = GenerateDynamicClass(UFGCustomizerSubCategory::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID)));
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

				UFGCustomizerSubCategory* InstClass = NewObject<UFGCustomizerSubCategory>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_Gen_USS_SwatchGroup_%d"), UniqueGroupID)), RF_MarkAsRootSet | RF_Public);

				if (InstClass)
				{
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


UFGFactoryCustomizationDescriptor_Swatch* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchDescriptor(int32 UniqueID, FText DisplayName, UFGCustomizerSubCategory* SwatchGroup)
{
	UClass* NewClass = GenerateDynamicClass(UFGFactoryCustomizationDescriptor_Swatch::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), UniqueID)));

	if (NewClass)
	{
		// Configurer le CDO
		UObject* tCDO = NewClass->GetDefaultObject();
		
		if (tCDO)
		{
			// Créer une instance temporaire pour servir de CDO
			UFGFactoryCustomizationDescriptor_Swatch* CDO = Cast<UFGFactoryCustomizationDescriptor_Swatch>(tCDO);

			if (CDO)
			{
				CDO->ID = UniqueID;
				CDO->mDisplayName = DisplayName;
				CDO->mCategory = UFGCustomizerCategory::StaticClass();
				if (SwatchGroup != nullptr) CDO->mSubCategories.Add(SwatchGroup->GetClass());
			}

			UFGFactoryCustomizationDescriptor_Swatch* InstClass = NewObject<UFGFactoryCustomizationDescriptor_Swatch>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_Gen_USS_SwatchDesc_%d"), UniqueID)), RF_MarkAsRootSet | RF_Public);
			
			if (InstClass)
			{
				InstClass->ID = UniqueID;
				InstClass->mDisplayName = DisplayName;
				InstClass->mCategory = UFGCustomizerCategory::StaticClass();
				if (SwatchGroup != nullptr) InstClass->mSubCategories.Add(SwatchGroup->GetClass());
			}

			if (!this->SwatchDescriptorArray.Contains(UniqueID))
			{
				this->SwatchDescriptorArray.Add(UniqueID, InstClass);
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
	UClass* NewClass = GenerateDynamicClass(UFGCustomizationRecipe::StaticClass(), FName(*FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), SwatchDescCDO->ID)));

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

			UFGCustomizationRecipe* InstClass = NewObject<UFGCustomizationRecipe>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_Gen_USS_SwatchRecipe_%d"), SwatchDescCDO->ID)), RF_MarkAsRootSet | RF_Public);

			if (InstClass)
			{
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


bool UUniversalSwatchSlotsWorldModule::GenerateNewSwatch(int32 UniqueGroupID, FText GroupDisplayName, float GroupPriority, int32 SwatchUniqueID, FText SwatchDisplayName, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe)
{
	if (this->SwatchDescriptorArray.Contains(SwatchUniqueID) || this->SwatchRecipeArray.Contains(SwatchUniqueID))
	{	// We can't overwrite existing swatch. (Well we could but I don't want to in order to not mess up evrything)

		SwatchGroup = nullptr;
		SwatchDescriptor = nullptr;
		SwatchRecipe = nullptr;
		return false;
	}

	UFGCustomizerSubCategory* SG = this->GenerateDynamicSwatchGroup(UniqueGroupID, GroupDisplayName, GroupPriority);
	UFGFactoryCustomizationDescriptor_Swatch* SD = this->GenerateDynamicSwatchDescriptor(SwatchUniqueID, SwatchDisplayName, SG);
	UFGCustomizationRecipe* SR = this->GenerateDynamicSwatchRecipe(SD);

	SwatchGroup = SG;
	SwatchDescriptor = SD;
	SwatchRecipe = SR;

	return true;
}


bool UUniversalSwatchSlotsWorldModule::GenerateNewSwatchUsingInfo(FUSSSwatch SwatchInformation, UFGCustomizerSubCategory*& SwatchGroup, UFGFactoryCustomizationDescriptor_Swatch*& SwatchDescriptor, UFGCustomizationRecipe*& SwatchRecipe)
{
	return this->GenerateNewSwatch(SwatchInformation.UniqueGroupID, SwatchInformation.GroupDisplayName, SwatchInformation.GroupPriority, SwatchInformation.SwatchUniqueID, SwatchInformation.SwatchDisplayName, SwatchGroup, SwatchDescriptor, SwatchRecipe);
}


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