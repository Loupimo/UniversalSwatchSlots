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

#include "USSBPLib.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlots, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlots)


bool UUniversalSwatchSlotsWorldModule::GenerateNewSwatch(int32 UniqueGroupID, FText GroupDisplayName, float GroupPriority, int32 SwatchUniqueID, FText SwatchDisplayName, float SwatchPriority, FLinearColor PrimaryColor, FLinearColor SecondaryColor, UUSSSwatchGroup*& SwatchGroup, UUSSSwatchDesc*& SwatchDescriptor, UUSSSwatchRecipe*& SwatchRecipe)
{
	FUSSSwatch SwatchInformation;
	//SwatchInformation.UniqueGroupID = UniqueGroupID;
	//SwatchInformation.GroupDisplayName = GroupDisplayName;
	//SwatchInformation.GroupPriority = GroupPriority;
	//SwatchInformation.SwatchUniqueID = SwatchUniqueID;
	//SwatchInformation.SwatchDisplayName = SwatchDisplayName;
	//SwatchInformation.PrimaryColour = PrimaryColor;
	//SwatchInformation.SecondaryColour = SecondaryColor;
	return this->USSSubsystem->GenerateNewSwatchUsingInfo(SwatchInformation, SwatchGroup, SwatchDescriptor, SwatchRecipe);
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

				//newPalette.PaletteName = (((UConfigPropertyString*)*association->SectionProperties.Find("PaletteName"))->Value);

				if (!this->Palettes.Contains(newPalette.PaletteName.ToString()))
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

							//	newPalette.AssociatedSessions.Add(newSession);
							}
							else
							{	// Ignore this session

								UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'Sessions' number % in 'Associations' at index %d. This association will be ignored."), sessionID, assoID);
							}

							sessionID++;
						}
					}

					//this->Palettes.Add(newPalette.PaletteName, newPalette);
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

				//for (FUSSSession AssociatedSession : newPalette->AssociatedSessions)
				{	// Find if the desired session is in this palette session association

				//	if (AssociatedSession.SessionName.Equals(this->SessionName))
					{	// Parse the palette as we need to apply it to the session

				//		UE_LOG(LogUniversalSwatchSlots, Display, TEXT("Find matching session \"%s\" for palette \"%s\". Loading palette."), *AssociatedSession.SessionName, *palName, *this->SessionName);

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
				//	else
					{	// Ignore the palette
						
				//		UE_LOG(LogUniversalSwatchSlots, Display, TEXT("Find session \"%s\" for palette \"%s\" but does not match desired session \"%s\". Ignoring palette for now."), *AssociatedSession.SessionName, *palName, *this->SessionName);
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

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'SwatchGroup' number %d in palette %s. Default will be used : %s."), GroupID, *OutPalette->PaletteName.ToString(), *groupName);
		}

		if (SwatchGroup->SectionProperties.Contains("Priority"))
		{	// A priority has been given

			groupPriority = ((UConfigPropertyFloat*)*SwatchGroup->SectionProperties.Find("Priority"))->Value;
		}
		else
		{	// No priority given, use default

			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Name' key found in 'SwatchGroup' number %d in palette %s. Default will be used : %s."), GroupID, *OutPalette->PaletteName.ToString(), *groupName);
		}

		if (SwatchGroup->SectionProperties.Contains("Swatches"))
		{	// The group has some swatches
			
			UConfigPropertyArray* Swatches = (UConfigPropertyArray*)*SwatchGroup->SectionProperties.Find("Swatches");

			for (UConfigProperty* SwatchProp : Swatches->Values)
			{	// Check all swatches

				if (this->USSSubsystem->ValidSlotIDs.Num() < 1)
				{
					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Can't have more than 255 swatch slots. Ignoring the rest."));
					return 255;
				}

				int32 swatchID = this->USSSubsystem->ValidSlotIDs[finalValidSlotID];
				if (swatchID >= 255)
				{	// ID 255 is used for the custom swatch

					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("Can't have more than 255 swatch slots. Ignoring the rest."));
					return 255;
				}

			//	if (!this->ParseSwatch(swatchID, (UConfigPropertySection*)SwatchProp, GroupID, groupName, groupPriority, OutPalette))
				{	// A swatch has been parsed. We can increment the swatch ID

					UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("There was a problem parsing swatch %d.Ignoring the rest."), swatchID);
				}
			//	else
				{
					finalValidSlotID++;
				}
			}

			return finalValidSlotID;
		}
		else
		{	// The group has no swatches, we could ignore it 
		
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Swatches' key found in 'SwatchGroup' %s in palette %s. The group will be ignored."), *groupName, *OutPalette->PaletteName.ToString());
		}
	}

	return finalValidSlotID;
}


bool UUniversalSwatchSlotsWorldModule::ParseSwatch(int32 SwatchID, UConfigPropertySection* Swatch, int32 GroupID, FString GroupName, float GroupPriority, FUSSGroup* OutGroup)
{
	/*if (Swatch)
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

		newSwatch.Name = FText::FromString(swatchName);

		if (Swatch->SectionProperties.Contains("Priority"))
		{	// A name has been given

			newSwatch.Priority = ((UConfigPropertyFloat*)*Swatch->SectionProperties.Find("Priority"))->Value;
		}
		else
		{	// No name given, build default

			newSwatch.Priority = 0.0f;
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Priority' key found in 'Swatch' number %d in palette %s in group %s. Default will be used : %f."), SwatchID, *OutPalette->PaletteName, *GroupName, 0.0f);
		}

		if (Swatch->SectionProperties.Contains("Primary"))
		{	// A primary color has been given

			newSwatch.PrimaryColour = UUSSBPLib::HexToLinearColor(((UConfigPropertyString*)*Swatch->SectionProperties.Find("Primary"))->Value);
		}
		else
		{	// No primary color given, use default

			newSwatch.PrimaryColour = UUSSBPLib::HexToLinearColor(defaultColor);
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Primary' key found in 'Swatch' %s in palette %s in group %s. Default will be used : %s."), *swatchName, *OutPalette->PaletteName, *GroupName, *defaultColor);
		}

		if (Swatch->SectionProperties.Contains("Secondary"))
		{	// A secondary color has been given

			newSwatch.SecondaryColour = UUSSBPLib::HexToLinearColor(((UConfigPropertyString*)*Swatch->SectionProperties.Find("Secondary"))->Value);
		}
		else
		{	// No secondary color given, use default
			
			newSwatch.SecondaryColour = UUSSBPLib::HexToLinearColor(defaultColor);
			UE_LOG(LogUniversalSwatchSlots, Warning, TEXT("No 'Secondary' key found in 'Swatch' %s in palette %s in group %s. Default will be used : %s."), *swatchName, *OutPalette->PaletteName, *GroupName, *defaultColor);
		}

		OutPalette->Swatches.Add(newSwatch);

		return true;
	}
	*/
	return false;
}


void UUniversalSwatchSlotsWorldModule::InitUSSGameWorldModule(bool CleanUpBeforeInit)
{
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (FGGameState)
	{
		this->SessionName = FGGameState->mReplicatedSessionName;
	}

	if (CleanUpBeforeInit)
	{
		AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);

		if (Subsystem && FGGameState)
		{
			int32 i = 28, j = i;
			if (this->IsUsingMoreSwatchSlots)
			{
				i = j = 48;
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
	
	this->USSSubsystem->RetrieveFreeColorSlotID();
}