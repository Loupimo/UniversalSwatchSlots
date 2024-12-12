#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "USS_Configuration1Struct.generated.h"

/* Struct generated from Mod Configuration Asset '/UniversalSwatchSlots/Config/Blueprints/USS_Configuration1' */
USTRUCT(BlueprintType)
struct FUSS_Configuration1Struct {
    GENERATED_BODY()

public:

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FUSS_Configuration1Struct GetActiveConfig(UObject* WorldContext) {
        FUSS_Configuration1Struct ConfigStruct{};
        FConfigId ConfigId{"UniversalSwatchSlots", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FUSS_Configuration1Struct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

