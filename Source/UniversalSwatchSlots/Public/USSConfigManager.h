// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Configuration/ConfigManager.h"

#include "USSConfigManager.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSConfigManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/** Registered configurations */
	UPROPERTY()
	TMap<FConfigId, FRegisteredConfigurationData> Configurations;
	//UModConfiguration;
public:

	void SaveConfigurationInternal(const FConfigId& ConfigId);
	
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void LoadConfigurationInternal(const FConfigId& ConfigId, URootConfigValueHolder* RootConfigValueHolder, bool bSaveOnSchemaChange);

	/** Returns configuration folder path used by config manager */
	static FString GetConfigurationFolderPath();

	/** Returns path to the provided configuration */
	static FString GetConfigurationFilePath(const FConfigId& ConfigId);
};
