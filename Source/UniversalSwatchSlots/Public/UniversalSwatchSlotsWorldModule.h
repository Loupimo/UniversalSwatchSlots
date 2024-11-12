// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/GameWorldModule.h"
#include "FGBuildableSubsystem.h"
#include "FGGameState.h"
#include "FGFactoryColoringTypes.h"
#include "USSCustomizerInterface.h"
#include "UniversalSwatchSlotsWorldModule.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUniversalSwatchSlotsWorldModule : public UGameWorldModule
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Schematic")
	void AddNewSwatch(FUSSSwatchInformation SwatchInformations);

	UFUNCTION(BlueprintCallable, Category = "Schematic")
	void ApplyColorData(AFGGameState* GameState, AFGBuildableSubsystem* BuildableSubSystem);
};
