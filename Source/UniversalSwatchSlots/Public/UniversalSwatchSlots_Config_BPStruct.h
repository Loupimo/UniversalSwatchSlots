#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "UniversalSwatchSlots_Config_BPStruct.generated.h"

struct FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes;
struct FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes_Default;

USTRUCT(BlueprintType)
struct FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes_Default {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    TArray<TArray<FString>> DefaultSwatchGroup{};
};

USTRUCT(BlueprintType)
struct FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes_Default Default{};
};

/* Struct generated from Mod Configuration Asset '/UniversalSwatchSlots/UniversalSwatchSlots_Config_BP' */
USTRUCT(BlueprintType)
struct FUniversalSwatchSlots_Config_BPStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    TArray<TArray<FString>> SaveGame{};

    UPROPERTY(BlueprintReadWrite)
    FUniversalSwatchSlots_Config_BPStruct_SwatchesPalettes SwatchesPalettes{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FUniversalSwatchSlots_Config_BPStruct GetActiveConfig(UObject* WorldContext) {
        FUniversalSwatchSlots_Config_BPStruct ConfigStruct{};
        FConfigId ConfigId{"UniversalSwatchSlots", "Swatches"};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FUniversalSwatchSlots_Config_BPStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

