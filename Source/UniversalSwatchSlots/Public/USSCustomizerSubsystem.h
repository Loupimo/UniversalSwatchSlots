// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGFactoryColoringTypes.h"
#include "FGSwatchGroup.h"
#include "Module/ModModule.h"
#include "USSContentCDOHelperInterface.h"
#include "USSCustomizerInterface.h"

#include "Module/WorldModule.h"
#include "Subsystems/WorldSubsystem.h"
#include "USSCustomizerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSCustomizerSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	virtual void    Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	bool GatherDefaultCollections();
	void GatherInterfaces();
	bool RegisterSwatchesInSubsystem(TArray<FUSSSwatchInformation> SwatchInformations);
	bool RegisterSwatchGroups(TMap<TSubclassOf<UFGSwatchGroup>, TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch>> Map);

	UFUNCTION(BlueprintCallable)
	void BeginForModule(UWorldModule* Module);

	UFUNCTION(BlueprintCallable)
	bool WasForModuleCalled(UWorldModule* Module) const {
		return mCalled.Contains(Module);
	};

	/**
	* Set Default Swatch to Swatch group (should call early before the player can place something)
	* otherwise it will crash
	*/
	UFUNCTION(BlueprintCallable, Category = "KMods|Customizer Subsystem")
	bool SetDefaultToSwatchGroup(TSubclassOf<UFGSwatchGroup> SwatchGroup, TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch> Swatch);

	UFUNCTION(BlueprintPure, Category = "KMods|Customizer Subsystem")
	FORCEINLINE TMap<uint8, TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch>> GetSwatchMap() const { return mSwatchIDMap; }

	bool Initialized = false;
	bool Gathered = false;

private:
	UPROPERTY()
	TArray<UWorldModule*> mCalled;

	TMap<uint8, TSubclassOf<UFGFactoryCustomizationDescriptor_Swatch>> mSwatchIDMap;
	TSubclassOf<UFGFactoryCustomizationCollection>                      mDefaultSwatchCollection;
	TSubclassOf<UFGFactoryCustomizationCollection>                      mDefaultMaterialCollection;
	TSubclassOf<UFGFactoryCustomizationCollection>                      mDefaultPatternCollection;
	TSubclassOf<UFGFactoryCustomizationCollection>                      mDefaultSkinCollection;
};
