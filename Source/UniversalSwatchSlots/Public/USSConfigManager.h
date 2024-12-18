// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "Engine/DataTable.h"
#include "UniversalSwatchSlotsDefinitions.h"

#include "USSConfigManager.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class UNIVERSALSWATCHSLOTS_API UUSSConfigManager : public UModConfiguration
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	TMap<FString, bool> DefaultConfName;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> ConfPaths;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* DefaultTable;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void FindConfigurationFiles(const FConfigId& ConfigurationId);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	void CreatesDefaultConfigurations(UGameInstance* GameInstance, const FConfigId& ConfigurationId);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	/** Returns configuration folder path used by config manager */
	static FString GetConfigurationFolderPath(const FConfigId& ConfigurationId);

	TSharedPtr<FJsonObject> ReadConfiguration(FString FilePath);
	bool WriteConfiguration(FString FilePath, TSharedPtr<FJsonObject> JsonObject);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	TMap<FString, FUSSPalette> ReadPalettesFromConfiguration(FString FilePath);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool SavePalettesToConfiguration(FString FilePath, TMap<FString, FUSSPalette> Palettes);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	FUSSPalette ReadPaletteFromConfiguration(FString FilePath);

	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool WritePaletteConfiguration(FString FilePath, FUSSPalette ToWrite);


	FString ExportDataTableToJson(UDataTable* DataTable);
};


