// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "FGSchematic.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "FGCustomizerCategory.h"
#include "Kismet/GameplayStatics.h"

DECLARE_LOG_CATEGORY_EXTERN(USSWorldModule, Log, All)

DEFINE_LOG_CATEGORY(USSWorldModule)


void UUniversalSwatchSlotsWorldModule::AddNewSwatches(TArray<FUSSSwatchInformation> SwatchInformations)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState) {
		for (FUSSSwatchInformation Swatch : SwatchInformations)
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
	// Create a dynamic derivated class
	UFGCustomizerSubCategory* NewClass = NewObject<UFGCustomizerSubCategory>(GetTransientPackage(), FName(*FString::Printf(TEXT("Gen_USS_SwatchGroup_%d"), UniqueGroupID)));
	if (NewClass)
	{
		// Modify CDO properties of the newly generated class
		UFGCustomizerSubCategory* CDO = Cast<UFGCustomizerSubCategory>(NewClass->GetClass()->GetDefaultObject());
		if (CDO)
		{
			CDO->mDisplayName = DisplayName;
			CDO->mMenuPriority = Priority;
		}
	}

	return NewClass;
}


UFGFactoryCustomizationDescriptor_Swatch* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchDescriptor(int32 UniqueID, FText DisplayName, UFGCustomizerSubCategory* SwatchGroup)
{
	// Create a dynamic derivated class
	UFGFactoryCustomizationDescriptor_Swatch* NewClass = NewObject<UFGFactoryCustomizationDescriptor_Swatch>(GetTransientPackage(), FName(*FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), UniqueID)));
	if (NewClass)
	{
		// Modify CDO properties of the newly generated class
		UFGFactoryCustomizationDescriptor_Swatch* CDO = Cast<UFGFactoryCustomizationDescriptor_Swatch>(NewClass->GetClass()->GetDefaultObject());
		if (CDO)
		{
			CDO->ID = UniqueID;
			CDO->mDisplayName = DisplayName;
			CDO->mCategory = UFGCustomizerCategory::StaticClass();
			CDO->mSubCategories.Add(SwatchGroup->GetClass());
		}
	}

	return NewClass;
}


UFGCustomizationRecipe* UUniversalSwatchSlotsWorldModule::GenerateDynamicSwatchRecipe(UFGFactoryCustomizationDescriptor_Swatch* SwatchDescriptor)
{
	
	UFGFactoryCustomizationDescriptor_Swatch* SwatchDescCDO = (UFGFactoryCustomizationDescriptor_Swatch *) SwatchDescriptor->GetClass()->GetDefaultObject();
	
	// Create a dynamic derivated class
	UFGCustomizationRecipe* NewClass = NewObject<UFGCustomizationRecipe>(GetTransientPackage(), FName(*FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), SwatchDescCDO->ID)));
	
	if (NewClass)
	{
		// Modify CDO properties of the newly generated class
		UFGCustomizationRecipe* CDO = Cast<UFGCustomizationRecipe>(NewClass->GetClass()->GetDefaultObject());
		if (CDO)
		{
			CDO->mDisplayName = SwatchDescCDO->mDisplayName;
			CDO->mCustomizationDesc = SwatchDescCDO->GetClass();
			CDO->mProducedIn.Add(this->BuildGunBPClass);
		}
	}

	return NewClass;
}