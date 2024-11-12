#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "UniversalSwatchSlotsConfigurationStruct.generated.h"

USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes_SwatchGroups_Swatches {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 ID{};

    UPROPERTY(BlueprintReadWrite)
    FString Name{};

    UPROPERTY(BlueprintReadWrite)
    FString Primary{};

    UPROPERTY(BlueprintReadWrite)
    FString Secondary{};
};

USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct_SaveGamePalette_SaveGame {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString Name{};
};

USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes_SwatchGroups {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString Name{};

    UPROPERTY(BlueprintReadWrite)
    TArray<FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes_SwatchGroups_Swatches> Swatches{};
};

USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct_SaveGamePalette {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString PaletteName{};

    UPROPERTY(BlueprintReadWrite)
    TArray<FUniversalSwatchSlotsConfigurationStruct_SaveGamePalette_SaveGame> SaveGame{};
};

USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FString PaletteName{};

    UPROPERTY(BlueprintReadWrite)
    TArray<FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes_SwatchGroups> SwatchGroups{};
};

/* Struct generated from Mod Configuration Asset '/UniversalSwatchSlots/UniversalSwatchSlotsConfiguration' */
USTRUCT(BlueprintType)
struct FUniversalSwatchSlotsConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    TArray<FUniversalSwatchSlotsConfigurationStruct_SaveGamePalette> SaveGamePalette{};

    UPROPERTY(BlueprintReadWrite)
    TArray<FUniversalSwatchSlotsConfigurationStruct_SwatchesPalettes> SwatchesPalettes{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FUniversalSwatchSlotsConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FUniversalSwatchSlotsConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"UniversalSwatchSlots", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FUniversalSwatchSlotsConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

