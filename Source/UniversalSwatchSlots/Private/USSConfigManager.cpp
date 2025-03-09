// Fill out your copyright notice in the Description page of Project Settings.


#include "USSConfigManager.h"

#include "Reflection/BlueprintReflectionLibrary.h"
#include "Misc/Paths.h"
#include "Util/SemVersion.h"
#include "TimerManager.h"
#include "Configuration/RootConfigValueHolder.h"
#include "Dom/JsonValue.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "Util/EngineUtil.h"
#include "HAL/FileManagerGeneric.h"
#include "Configuration/Properties/ConfigPropertyString.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUSSConfigManager, Log, All)
DEFINE_LOG_CATEGORY(LogUSSConfigManager)

void UUSSConfigManager::InitConfigManager()
{
    this->FindConfigurationFiles();

    for (auto Path : this->ConfPaths)
    {   // Extract all available palette

        this->ConfPalettes.Append(this->ReadPalettesFromConfiguration(Path.Key));
    }

    this->ConfPaths.Empty();

    FUSSPalette* DefaultPalette = this->DefaultTable->FindRow<FUSSPalette>(this->DefaultTable->GetRowNames()[0], nullptr);
    FText DefaultConf = DefaultPalette->PaletteName;
    bool defaultPalFound = false; 
    FString ConfPath = this->GetConfigurationFolderPath();

    for (auto Path : this->ConfPalettes)
    {   // Rebuild the path map in order to be coherent with the palette. Nothing should change except if several palette were saved inside a same file

        if (DefaultPalette->PaletteName.ToString() == Path.Value.PaletteName.ToString())
        {
            defaultPalFound = true;
        }
        this->ConfPaths.Add(ConfPath + Path.Value.PaletteName.ToString(), false);
    }

    bool activePaletteFound = this->FixActivePalette();

    // Check if we should active the default configuration or not
    if (this->RootSection)
    {
        if (this->RootSection->SectionProperties.Contains("UserDefined"))
        {
            FString userDefined = ((UConfigPropertyString*)*this->RootSection->SectionProperties.Find("UserDefined"))->Value;
            this->ShouldActivateDefault = userDefined == FString("No") ? true : false;
        }
        else
        {
            this->ShouldActivateDefault = true;
        }
    }

    if (!defaultPalFound)
    {   // We need to add the default palette

        if (!activePaletteFound && this->ShouldActivateDefault)
        {   // We need to activate the default configuration

            DefaultPalette->IsActive = true;
            ((UConfigPropertyString*)*this->RootSection->SectionProperties.Find("ActivePalette"))->Value = DefaultPalette->PaletteName.ToString();
            ((UConfigPropertyString*)*this->RootSection->SectionProperties.Find("ActivePalette"))->MarkDirty();
        }
        this->ConfPalettes.Add(this->GetConfigurationFolderPath() + DefaultPalette->PaletteName.ToString(), *DefaultPalette);
    }
}


bool UUSSConfigManager::FixActivePalette()
{
    bool activeFound = false;
    for (auto & currConf : this->ConfPalettes)
    {   // Check all palette

        if (currConf.Value.IsActive)
        {   // The current palette is active

            if (activeFound)
            {   // We already have an active palette
                
                currConf.Value.IsActive = false;
            }
            else
            {   // This is the first one

                activeFound = true;
                ((UConfigPropertyString*)*this->RootSection->SectionProperties.Find("ActivePalette"))->Value = currConf.Value.PaletteName.ToString();
                ((UConfigPropertyString*)*this->RootSection->SectionProperties.Find("ActivePalette"))->MarkDirty();
            }
        }
    }

    return activeFound;
}


void UUSSConfigManager::FindConfigurationFiles()
{
    //Determine configuration path and try to read it to string if it exists
    const FString ConfigurationFolderPath = GetConfigurationFolderPath();
    FString ConfigurationFilePath = ConfigurationFolderPath;
    FJsonSerializableArray arr;
    FFileManagerGeneric fm = FFileManagerGeneric();
    fm.FindFiles(arr, *ConfigurationFilePath, nullptr);

    for (FString filePath : arr)
    {   // Check for all configurations

        ConfigurationFilePath = ConfigurationFolderPath + filePath;
        this->ConfPaths.Add(ConfigurationFilePath, false);
    }
}


void UUSSConfigManager::MarkConfigurationAsDirty(FUSSPalette ToMark)
{
    FString ConfPath = this->GetConfigurationFolderPath() + ToMark.PaletteName.ToString();

    FUSSPalette* exist = this->ConfPalettes.Find(ConfPath);
    bool* mark = this->ConfPaths.Find(ConfPath);

    if (exist == nullptr)
    {   // The configuration doesn't exist yet

        this->ConfPalettes.Add(ConfPath, ToMark);
        this->ConfPaths.Add(ConfPath, true);
        this->ConfToDelete.Remove(ConfPath);
    }
    else
    {   // The configuration already exist

        *exist = ToMark;
    }

    if (mark == nullptr)
    {
        this->ConfPaths.Add(ConfPath, true);
        this->ConfToDelete.Remove(ConfPath);
    }
    else
    {
        *mark = true;
        this->ConfToDelete.Remove(ConfPath);
    }
}


void UUSSConfigManager::MarkConfigurationAsDeleted(FString ConfPath)
{
    ConfPath = this->GetConfigurationFolderPath() + ConfPath;
    this->ConfPaths.Remove(ConfPath);
    this->ConfPalettes.Remove(ConfPath);
    //this->ConfToDelete.Add(ConfPath);
    FPlatformFileManager::Get().GetPlatformFile().DeleteFile(ConfPath.GetCharArray().GetData());
}


TMap<FString, FUSSPalette> UUSSConfigManager::ReadPalettesFromConfiguration(FString FilePath)
{
    //Load file contents into the string for parsing
    FString JsonTextString;
    if (!FFileHelper::LoadFileToString(JsonTextString, *FilePath)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to load configuration file from %s"), *FilePath);
        return TMap<FString, FUSSPalette>();
    }

    // Create the data table
    UDataTable* Table = NewObject<UDataTable>();
    Table->RowStruct = FUSSPalette::StaticStruct();

    // Populate the table
    Table->CreateTableFromJSONString(JsonTextString);

    // Retrieve rows
    TArray<FName> RowNames = Table->GetRowNames();

    FString ConfPath = this->GetConfigurationFolderPath();

    // Populate the returned map
    TMap<FString, FUSSPalette> palettes;

    for (FName RowName : RowNames)
    {
        FUSSPalette* palette = Table->FindRow<FUSSPalette>(RowName, nullptr);

        if (palette)
        {   // The palette is valid

            palettes.Add(this->GetConfigurationFolderPath() + palette->PaletteName.ToString(), *palette);
        }
        else
        {   // There was a problem with this palette

            UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to parse palette %s from configuration file %s"), RowName, *FilePath);
        }
    }

    return palettes;
}



void UUSSConfigManager::SaveAndDeleteConfigurations()
{
    for (auto SaveConf : this->ConfPaths)
    {   // Save all configuration marked as dirty

        if (SaveConf.Value == true)
        {   // We need to save the configuration

            FUSSPalette* ToSave = this->ConfPalettes.Find(SaveConf.Key);
            if (ToSave)
            {
                UE_LOG(LogUSSConfigManager, Verbose, TEXT("Saving configuration %s"), *SaveConf.Key);

                this->SavePaletteToConfiguration(SaveConf.Key, *ToSave);
            }
        }
    }

    /*for (auto DeleteConf : this->ConfToDelete)
    {   // Delete all the given configuration

        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(DeleteConf.GetCharArray().GetData());
    }*/
}


bool UUSSConfigManager::SavePaletteToConfiguration(FString FilePath, FUSSPalette Palette)
{
    // Retrieve rows
    TArray<FString> RowNames;

    // Create the data table
    UDataTable* Table = NewObject<UDataTable>();
    Table->RowStruct = FUSSPalette::StaticStruct();

    // Populate the returned map
    Table->AddRow(FName(Palette.PaletteName.ToString()), Palette);
	
	FString TableString = this->ExportDataTableToJson(Table);

    // Make sure configuration directory exists
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

    if (!FFileHelper::SaveStringToFile(TableString, *FilePath))
    {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to save configuration file to %s"), *FilePath);
        return false;
    }

    return true;
}


FString UUSSConfigManager::GetConfigurationFolderPath()
{
    return FPaths::ProjectDir() + TEXT("Configs/") + FString::Printf(TEXT("%s/"), *this->ConfigId.ModReference);
}


FString UUSSConfigManager::ExportDataTableToJson(UDataTable* DataTable)
{
    if (!DataTable) return TEXT("[]");

    TArray<FName> RowNames = DataTable->GetRowNames();
    TArray<TSharedPtr<FJsonValue>> JsonArray;

    for (const FName& RowName : RowNames)
    {
        // Obtenir la structure correspondant à la ligne
        uint8* RowData = DataTable->FindRowUnchecked(RowName);
        if (RowData)
        {
            TSharedPtr<FJsonObject> TempJsonObject = MakeShared<FJsonObject>();

            // Conversion initiale en JSON
            FJsonObjectConverter::UStructToJsonObject(DataTable->RowStruct, RowData, TempJsonObject.ToSharedRef(), 0, 0);

            // Correction des clés pour correspondre aux noms d'origine
            TSharedPtr<FJsonObject> FixedJsonObject = FixJsonKeys(TempJsonObject, DataTable->RowStruct);

            // Ajout manuel du champ "Name" au premier niveau de l'objet
            FixedJsonObject->SetStringField("Name", RowName.ToString());

            JsonArray.Add(MakeShared<FJsonValueObject>(FixedJsonObject));
        }
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonArray, Writer);

    return OutputString;
}

// Fonction récursive pour corriger les clés des objets imbriqués
TSharedPtr<FJsonObject> UUSSConfigManager::FixJsonKeys(const TSharedPtr<FJsonObject>& JsonObject, UScriptStruct* StructType)
{
    TSharedPtr<FJsonObject> CorrectedJsonObject = MakeShared<FJsonObject>();

    for (TFieldIterator<FProperty> It(StructType); It; ++It)
    {
        FProperty* Property = *It;
        FString OriginalName = Property->GetName(); // Nom réel de la propriété

        if (JsonObject->HasField(OriginalName))
        {
            // Vérifier si c'est une structure imbriquée
            if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
            {
                TSharedPtr<FJsonObject> SubObject = JsonObject->GetObjectField(OriginalName);
                if (SubObject.IsValid())
                {
                    CorrectedJsonObject->SetObjectField(OriginalName, FixJsonKeys(SubObject, StructProp->Struct));
                    continue;
                }
            }

            // Vérifier si c'est un tableau de structures
            if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
            {
                if (FStructProperty* ElemStructProp = CastField<FStructProperty>(ArrayProp->Inner))
                {
                    TArray<TSharedPtr<FJsonValue>> FixedArray;
                    TArray<TSharedPtr<FJsonValue>> OriginalArray = JsonObject->GetArrayField(OriginalName);

                    for (const auto& ArrayElement : OriginalArray)
                    {
                        if (ArrayElement->AsObject().IsValid())
                        {
                            FixedArray.Add(MakeShared<FJsonValueObject>(
                                FixJsonKeys(ArrayElement->AsObject(), ElemStructProp->Struct)));
                        }
                        else
                        {
                            FixedArray.Add(ArrayElement); // Conserver l'élément s'il n'est pas un objet
                        }
                    }

                    CorrectedJsonObject->SetArrayField(OriginalName, FixedArray);
                    continue;
                }
            }

            // Copier la valeur telle quelle
            CorrectedJsonObject->SetField(OriginalName, JsonObject->TryGetField(OriginalName));
        }
    }

    return CorrectedJsonObject;
}