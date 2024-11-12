// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"
#include "FGSchematic.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Swatch/USS_Recipe_CPP.h"
#include "Kismet/GameplayStatics.h"

DECLARE_LOG_CATEGORY_EXTERN(USSWorldModule, Log, All)

DEFINE_LOG_CATEGORY(USSWorldModule)


void UUniversalSwatchSlotsWorldModule::AddNewSwatch(FUSSSwatchInformation SwatchInformations)
{
	/*AFGGameState* GS = (AFGGameState*)(GameState->GetClass()->GetDefaultObject());//Cast<AFGGameState>(AFGGameState::StaticClass()->GetDefaultObject(true));
	AFGBuildableSubsystem* BSS = (AFGBuildableSubsystem*) (BuildableSubSystem->GetClass()->GetDefaultObject()); //Cast<AFGBuildableSubsystem>(AFGBuildableSubsystem::StaticClass()->GetDefaultObject(true));


    UE_LOG(USSWorldModule, Log, TEXT("Swatch index : %d"), SwatchID);
	// create now Slots
	for (uint8 i = 0; i == SwatchID; i++)
	{
		FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot();
		NewColourSlot.PrimaryColor = FColor::FromHex("FFE100FF");
		NewColourSlot.SecondaryColor = FColor::FromHex("1700FFFF");

		if (!BSS->mColorSlots_Data.IsValidIndex(i))
		{
			BSS->mColorSlots_Data.Add(NewColourSlot);
			BSS->SetColorSlot_Data(i, NewColourSlot);  // need? it now always work...

			GS->mBuildingColorSlots_Data.Add(NewColourSlot);
			GS->Server_SetBuildingColorDataForSlot(i, NewColourSlot);  // need? it now always work...

			// mark Colours as Dirty for update colours
		}

		BSS->SetColorSlot_Data(i, NewColourSlot);  // need? it now always work...
		BSS->mColorSlotsAreDirty = true;
		//GS->Server_SetBuildingColorDataForSlot(i, NewColourSlot);  // need? it now always work...
		//if (!GS->mBuildingColorSlots_Data.IsValidIndex(i))
		//{
		//}

		UE_LOG(USSWorldModule, Log, TEXT("Add new color slot at index : %d"), SwatchID);
	}*/

	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	//UKBFLContentCDOHelperSubsystem* CDOHelperSubsystem = UKBFLContentCDOHelperSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState) {
		//for (FKBFLSwatchInformation Swatch : SwatchInformations) {
			if (SwatchInformations.mSwatch) {
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = SwatchInformations.mSwatch.GetDefaultObject();
				if (SwatchDefauls) {
					uint8 ColourIndex = SwatchDefauls->ID;
					SwatchDefauls->mMenuPriority = static_cast<float>(ColourIndex);
					//if (!mSwatchIDMap.Find(ColourIndex)) {
						/*if (mDefaultSwatchCollection) {
							UFGFactoryCustomizationCollection* Default = CDOHelperSubsystem->GetAndStoreDefaultObject_Native<UFGFactoryCustomizationCollection>(mDefaultSwatchCollection);
							if (IsValid(Default)) {
								Default->mCustomizations.AddUnique(Swatch.mSwatch);
							}
						}*/

						if (!Subsystem->mColorSlots_Data.IsValidIndex(ColourIndex)) {
							UE_LOG(USSWorldModule, Log, TEXT("Try to add new color Slot at index, %d - %d"), ColourIndex, Subsystem->mColorSlots_Data.Num());
							for (uint8 i = Subsystem->mColorSlots_Data.Num(); i <= ColourIndex; ++i) {
								// Defaults
								FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(SwatchInformations.mPrimaryColour, SwatchInformations.mSecondaryColour);

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
						if (!FGGameState->mBuildingColorSlots_Data.IsValidIndex(ColourIndex)) {
							FGGameState->mBuildingColorSlots_Data.SetNum(ColourIndex + 1, false);
							FGGameState->mBuildingColorSlots_Data[ColourIndex] = Subsystem->mColorSlots_Data[ColourIndex];
							FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
							UE_LOG(USSWorldModule, Log, TEXT("write color again to gamestate: %d / %d"), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
						}

						//mSwatchIDMap.Add(ColourIndex, Swatch.mSwatch);
						FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
						Subsystem->mColorSlotsAreDirty = true;
						UE_LOG(USSWorldModule, Log, TEXT("Swatch found and success: %d > %s (%d/%d)"), ColourIndex, *SwatchInformations.mSwatch->GetName(), FGGameState->mBuildingColorSlots_Data.Num(), Subsystem->mColorSlots_Data.Num());
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
		//}

		TArray<FFactoryCustomizationColorSlot> ColorSlots = Subsystem->mColorSlots_Data;
		UE_LOG(USSWorldModule, Error, TEXT("Slots: %d"), ColorSlots.Num());
		FGGameState->SetupColorSlots_Data(ColorSlots);
		//FGGameState->Init();
		Subsystem->mColorSlotsAreDirty = true;

		return;
	}

	if (FGGameState) {
		//for (FKBFLSwatchInformation Swatch : SwatchInformations) {
			if (SwatchInformations.mSwatch) {
				UFGFactoryCustomizationDescriptor_Swatch* SwatchDefauls = SwatchInformations.mSwatch.GetDefaultObject();
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
		//}
	}

	UE_LOG(USSWorldModule, Error, TEXT("Cannot load AFGBuildableSubsystem for Swatches"));
	return;
}

void UUniversalSwatchSlotsWorldModule::ApplyColorData(AFGGameState* GameState, AFGBuildableSubsystem* BuildableSubSystem)
{
	/*FGGameState* GS = (AFGGameState*)(GameState->GetClass()->GetDefaultObject());//Cast<AFGGameState>(AFGGameState::StaticClass()->GetDefaultObject(true));
	AFGBuildableSubsystem* BSS = (AFGBuildableSubsystem*)(BuildableSubSystem->GetClass()->GetDefaultObject()); //Cast<AFGBuildableSubsystem>(AFGBuildableSubsystem::StaticClass()->GetDefaultObject(true));

	GS->SetupColorSlots_Data(BSS->mColorSlots_Data);*/
}


