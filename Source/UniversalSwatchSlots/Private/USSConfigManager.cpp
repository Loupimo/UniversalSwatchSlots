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

DECLARE_LOG_CATEGORY_EXTERN(LogUSSConfigManager, Log, All)
DEFINE_LOG_CATEGORY(LogUSSConfigManager)

const TCHAR* SMLConfigModVersionField = TEXT("SML_ModVersion_DoNotChange");

void UUSSConfigManager::SaveConfigurationInternal(const FConfigId& ConfigId) {
    const FRegisteredConfigurationData& ConfigurationData = Configurations.FindChecked(ConfigId);

    const URootConfigValueHolder* RootValue = ConfigurationData.RootValue;
    URawFormatValue* RawFormatValue = nullptr;//RootValue->GetWrappedValue()->Serialize(GetTransientPackage());
    checkf(RawFormatValue, TEXT("Root RawFormatValue returned NULL for config %s"), *ConfigId.ModReference);

    //Root value should always be JsonObject, since root property is section property
    const TSharedPtr<FJsonValue> JsonValue = FJsonRawFormatConverter::ConvertToJson(RawFormatValue);
    check(JsonValue->Type == EJson::Object);
    TSharedRef<FJsonObject> UnderlyingObject = JsonValue->AsObject().ToSharedRef();

    //Record mod version so we can keep file system file schema up to date
    FModInfo ModInfo;
    UModLoadingLibrary* ModLoadingLibrary = GetGameInstance()->GetSubsystem<UModLoadingLibrary>();

    if (ModLoadingLibrary->GetLoadedModInfo(ConfigId.ModReference, ModInfo)) {
        const FString ModVersion = ModInfo.Version.ToString();
        UnderlyingObject->SetStringField(SMLConfigModVersionField, ModVersion);
    }

    //Serialize resulting JSON to string
    FString JsonOutputString;
    const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonOutputString);
    FJsonSerializer::Serialize(UnderlyingObject, JsonWriter);

    //Write configuration into the file system now at the generated path
    const FString ConfigurationFilePath = GetConfigurationFilePath(ConfigId);
    //Make sure configuration directory exists
    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(ConfigurationFilePath));

    if (!FFileHelper::SaveStringToFile(JsonOutputString, *ConfigurationFilePath)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to save configuration file to %s"), *ConfigurationFilePath);
        return;
    }
    UE_LOG(LogUSSConfigManager, Display, TEXT("Saved configuration to %s"), *ConfigurationFilePath);
}

void UUSSConfigManager::LoadConfigurationInternal(const FConfigId& ConfigId, URootConfigValueHolder* RootConfigValueHolder, bool bSaveOnSchemaChange) {
    //Determine configuration path and try to read it to string if it exists
    const FString ConfigurationFilePath = GetConfigurationFolderPath();//GetConfigurationFilePath(ConfigId);
    const FString Co = ".cfg";
    FJsonSerializableArray arr;
    FFileManagerGeneric fm = FFileManagerGeneric();
    fm.FindFiles(arr, *ConfigurationFilePath, nullptr);

    //Check if configuration file exists, and if it doesn't, return early, optionally writing defaults
    if (!IFileManager::Get().FileExists(*ConfigurationFilePath)) {
        if (bSaveOnSchemaChange) {
            SaveConfigurationInternal(ConfigId);
        }
        return;
    }

    //Load file contents into the string for parsing
    FString JsonTextString;
    if (!FFileHelper::LoadFileToString(JsonTextString, *ConfigurationFilePath)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to load configuration file from %s"), *ConfigurationFilePath);
        return;
    }

    //Try to parse it as valid JSON now
    const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonTextString);
    TSharedPtr<FJsonObject> JsonObject;
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
        UE_LOG(LogUSSConfigManager, Error, TEXT("Failed to parse configuration file %s"), *ConfigurationFilePath);
        //TODO maybe rename it and write default values instead?
        return;
    }

    //Convert JSON tree into the raw value tree and feed it to root section value
    const TSharedRef<FJsonValue> RootValue = MakeShareable(new FJsonValueObject(JsonObject));
    URawFormatValue* RawFormatValue = FJsonRawFormatConverter::ConvertToRawFormat(this, RootValue);
    //RootConfigValueHolder->GetWrappedValue()->Deserialize(RawFormatValue);

    UE_LOG(LogUSSConfigManager, Display, TEXT("Successfully loaded configuration from %s"), *ConfigurationFilePath);

    //Check that mod version matches if we are allowed to overwrite files
    FModInfo ModInfo;
    UModLoadingLibrary* ModLoadingLibrary = GetGameInstance()->GetSubsystem<UModLoadingLibrary>();

    if (ModLoadingLibrary->GetLoadedModInfo(ConfigId.ModReference, ModInfo)) {
        const FString ModVersion = ModInfo.Version.ToString();
        FString FileVersion;
        if (JsonObject->HasTypedField<EJson::String>(SMLConfigModVersionField)) {
            FileVersion = JsonObject->GetStringField(SMLConfigModVersionField);
        }
        //Overwrite file if schema version doesn't match loaded mod version
        if (bSaveOnSchemaChange && FileVersion != ModVersion) {
            UE_LOG(LogUSSConfigManager, Display, TEXT("Refreshing configuration file %s"), *ConfigurationFilePath);
            SaveConfigurationInternal(ConfigId);
        }
    }
}


FString UUSSConfigManager::GetConfigurationFolderPath() {
    return FPaths::ProjectDir() + TEXT("Configs/");
}

FString UUSSConfigManager::GetConfigurationFilePath(const FConfigId& ConfigId) {
    const FString ConfigDirectory = GetConfigurationFolderPath();
    if (ConfigId.ConfigCategory == TEXT("")) {
        //Category is empty, that means mod has only one configuration file
        return ConfigDirectory + FString::Printf(TEXT("%s.cfg"), *ConfigId.ModReference);
    }
    //We have a category, so mod reference is a folder and category is a file name
    return ConfigDirectory + FString::Printf(TEXT("%s/%s.cfg"), *ConfigId.ModReference, *ConfigId.ConfigCategory);
}