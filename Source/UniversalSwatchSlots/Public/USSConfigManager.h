// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ModConfiguration.h"
#include "Engine/DataTable.h"
#include "UniversalSwatchSlotsDefinitions.h"
#include "Configuration/RawFileFormat/Json/JsonRawFormatConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"

#include "USSConfigManager.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class UNIVERSALSWATCHSLOTS_API UUSSConfigManager : public UModConfiguration
{
	GENERATED_BODY()

public:

	/* The key represents the configuration path and the value tells if the file is dirty and needs to be saved or not. */
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, bool> ConfPaths;

	/* The key represents the configuration path and the value the palette associated to it. */
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FUSSPalette> ConfPalettes;

	/* The table that represent the structure used to save / load the configuration files. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataTable> DefaultTable;

public:

	/**
	 * Initialize the configuration manager by loading all configurations from the UniversalSwatchSlots folder.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void InitConfigManager();

	/**
	 * Mark the configuration matching the given palette as dirty.
	 *
	 * Note : if the configuration doesn't exist it will add it to the ConfPaths and ConfPalettes maps.
	 * 
	 * @param	OldName		The palette to name before the palette was dirty.
	 * @param	ToMark		The palette to mark as dirty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void MarkConfigurationAsDirty(FText OldName, FUSSPalette ToMark);
	
	/**
	 * Delete the given configuration.
	 *
	 * @param	ToDelete		The palette to deleted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void DeleteConfiguration(FString ToDelete);
	
	/**
	 * Get the configuration folder path for this mod.
	 *
	 * @return	The configuration folder path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	FString GetConfigurationFolderPath();

	/**
	 * Save all the files marked as dirty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void SaveConfigurations();

	/**
	 * Save the given palette to the desired configuration file.
	 * 
	 * @param	FilePath	The file path to save the palette to.
	 * @param	Palette		The palette to save.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	bool SavePaletteToConfiguration(FString FilePath, FUSSPalette Palette);

	/**
	 * Read the palettes contained in the given configuration file.
	 *
	 * @param	FilePath	The file path of the configuration to read.
	 */
	TMap<FString, FUSSPalette> ReadPalettesFromConfiguration(FString FilePath);

	/**
	 * Find all the configuration files related to this mod.
	 */
	void FindConfigurationFiles();

	/**
	 * Convert the given data table to a JSON string.
	 * 
	 * @param	DataTable	The data table to convert.
	 * 
	 * @return	The JSON string representation of the given data table.
	 */
	FString ExportDataTableToJson(UDataTable* DataTable);

	TSharedPtr<FJsonObject> FixJsonKeys(const TSharedPtr<FJsonObject>& JsonObject, UScriptStruct* StructType);

	/**
	 * Check the avaliable palette and check for multiple active palette. If some are found only the first will be considered as active.
	 * 
	 * @return	True if an active palette has been found, false otherwise.
	 */
	bool FixActivePalette();

};


