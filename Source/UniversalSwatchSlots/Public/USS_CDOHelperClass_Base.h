// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "FGRecipe.h"
#include "Equipment/FGBuildGun.h"

#include "USS_CDOHelperClass_Base.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, abstract, DefaultToInstanced)
class UNIVERSALSWATCHSLOTS_API UUSS_CDOHelperClass_Base : public UObject
{
	GENERATED_BODY()
	
public:
#if WITH_ENGINE
	virtual UWorld* GetWorld() const override;
#endif

	UFUNCTION(BlueprintCallable)
	virtual void DoCDO();

	UFUNCTION(BlueprintCallable)
	virtual TArray<UClass*> GetClasses();

	UFUNCTION(BlueprintCallable)
	virtual void GetDefaultObjects(TArray<UObject*>& CDOs);

	UFUNCTION(BlueprintPure)
	static bool IsValidSoftClass(TSoftClassPtr<UObject> Class);

	UFUNCTION(BlueprintPure)
	bool HasAuth();

	UFUNCTION(BlueprintPure)
	static bool ContainBuildGun(TSubclassOf<UFGRecipe> Subclass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool         ExecuteAllowed() const;
	virtual bool ExecuteAllowed_Implementation() const;

	UFUNCTION(BlueprintNativeEvent)
	void ExecuteBlueprintCDO();

	virtual void ExecuteBlueprintCDO_Implementation() {
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ModifyValues();

	virtual void ModifyValues_Implementation() {
	}

	UPROPERTY(BlueprintReadWrite)
	class UUSSContentCDOHelperSubsystem* mSubsystem = nullptr;
};
