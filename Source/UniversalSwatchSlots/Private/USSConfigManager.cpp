// Fill out your copyright notice in the Description page of Project Settings.


#include "USSConfigManager.h"

#include "Reflection/BlueprintReflectionLibrary.h"
#include "Misc/Paths.h"
#include "Util/SemVersion.h"
#include "TimerManager.h"
#include "Configuration/RootConfigValueHolder.h"
#include "Configuration/RawFileFormat/Json/JsonRawFormatConverter.h"
#include "Dom/JsonValue.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Util/EngineUtil.h"
#include "HAL/FileManagerGeneric.h"
#include "JsonObjectConverter.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUSSConfigManager, Log, All)
DEFINE_LOG_CATEGORY(LogUSSConfigManager)

void UUSSConfigManager::FindConfigurationFiles(const FConfigId& ConfigurationId)
{
    //Determine configuration path and try to read it to string if it exists
    const FString ConfigurationFolderPath = GetConfigurationFolderPath(ConfigurationId);
    FString ConfigurationFilePath = ConfigurationFolderPath;
    FJsonSerializableArray arr;
    FFileManagerGeneric fm = FFileManagerGeneric();
    fm.FindFiles(arr, *ConfigurationFilePath, nullptr);

    for (FString filePath : arr)
    {   // Check for all configurations

        ConfigurationFilePath = ConfigurationFolderPath + filePath;

        bool* res = this->DefaultConfName.Find(filePath);
        if (res)
        {   // We have found the default configuration

            *res = true;
        }
        this->ConfPaths.Add(ConfigurationFilePath);
    }
}


void UUSSConfigManager::CreatesDefaultConfigurations(UGameInstance* GameInstance, const FConfigId& ConfigurationId)
{

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

    // Populate the returned map
    TMap<FString, FUSSPalette> palettes;

    for (FName RowName : RowNames)
    {
        FUSSPalette* palette = Table->FindRow<FUSSPalette>(RowName, nullptr);

        if (palette)
        {   // The palette is valid

            palettes.Add(RowName.ToString(), *palette);
        }
        else
        {   // There was a problem with this palette

            UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to parse palette %s from configuration file %s"), RowName, *FilePath);
        }
    }

    return palettes;
}


FUSSPalette UUSSConfigManager::ReadPaletteFromConfiguration(FString FilePath)
{
    TSharedPtr<FJsonObject> JsonObject = this->ReadConfiguration(FilePath);

    if (JsonObject == nullptr)
    {
        return FUSSPalette();
    }

    FUSSPalette newPalette;

    if (!FJsonObjectConverter::JsonObjectToUStruct<FUSSPalette>(JsonObject.ToSharedRef(), &newPalette))
    {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to parse configuration file %s"), *FilePath);
        return FUSSPalette();
    }

    return newPalette;
}


bool UUSSConfigManager::SavePalettesToConfiguration(FString FilePath, TMap<FString, FUSSPalette> Palettes)
{
    // Retrieve rows
    TArray<FString> RowNames;
    Palettes.GetKeys(RowNames);

    // Create the data table
    UDataTable* Table = NewObject<UDataTable>();
    Table->RowStruct = FUSSPalette::StaticStruct();

    // Populate the returned map
    TMap<FString, FUSSPalette> palettes;

    for (FString RowName : RowNames)
    {
        Table->AddRow(FName(RowName), Palettes[RowName]);
    }
	
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


bool UUSSConfigManager::WritePaletteConfiguration(FString FilePath, FUSSPalette ToWrite)
{
    TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(ToWrite);
    
    if (JsonObject == nullptr)
    {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to save configuration file to %s"), *FilePath);
        return false;
    }

    this->WriteConfiguration(FilePath, JsonObject);
    return true;
}


TSharedPtr<FJsonObject> UUSSConfigManager::ReadConfiguration(FString FilePath)
{
    //Load file contents into the string for parsing
    FString JsonTextString;
    if (!FFileHelper::LoadFileToString(JsonTextString, *FilePath)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to load configuration file from %s"), *FilePath);
        return nullptr;
    }

    //Try to parse it as valid JSON now
    const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonTextString);
    TSharedPtr<FJsonObject> JsonObject;
    
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to parse configuration file %s"), *FilePath);
        //TODO maybe rename it and write default values instead?
        return nullptr;
    }

    return JsonObject;
}


bool UUSSConfigManager::WriteConfiguration(FString FilePath, TSharedPtr<FJsonObject> JsonObject)
{
    //Serialize resulting JSON to string
    FString JsonOutputString;
    const TSharedRef<TJsonWriter<>> JsonWriter2 = TJsonWriterFactory<>::Create(&JsonOutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter2);

    //Make sure configuration directory exists
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

    if (!FFileHelper::SaveStringToFile(JsonOutputString, *FilePath)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to save configuration file to %s"), *FilePath);
        return false;
    }
    UE_LOG(LogUSSConfigManager, Display, TEXT("Saved configuration to %s"), *FilePath);

    return true;
}


FString UUSSConfigManager::GetConfigurationFolderPath(const FConfigId& ConfigurationId)
{
    return FPaths::ProjectDir() + TEXT("Configs/") + FString::Printf(TEXT("%s/"), *ConfigurationId.ModReference);
}

FString UUSSConfigManager::ExportDataTableToJson(UDataTable* DataTable)
{
    if (!DataTable) return TEXT("{}");

    TArray<FName> RowNames = DataTable->GetRowNames();
    TArray<TSharedPtr<FJsonValue>> JsonArray;

    for (const FName& RowName : RowNames)
    {
        // Obtenir la structure correspondant à la ligne
        uint8* RowData = DataTable->FindRowUnchecked(RowName);
        if (RowData)
        {
            // Créer un objet JSON pour chaque ligne
            TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
            FJsonObjectConverter::UStructToJsonObject(DataTable->RowStruct, RowData, JsonObject.ToSharedRef(), 0, 0);
            JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));
        }
    }

    // Convertir l'ensemble en chaîne JSON
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetArrayField("Rows", JsonArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    return OutputString;
}