// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGSchematic.h"
#include "Module/ModModule.h"
#include "Resources/FGResourceDescriptor.h"
#include "Resources/FGResourceNode.h"
#include "UObject/Interface.h"
#include "USS_CDOHelperClass_Base.h"
#include "UObject/NoExportTypes.h"
#include "USSContentCDOHelperInterface.generated.h"

USTRUCT(BlueprintType)
struct FUSSItemArray {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UFGItemDescriptor>> mItems = {};
};

USTRUCT(BlueprintType)
struct FUSSPhases {
	GENERATED_BODY()

	FUSSPhases() {
		mCalledPhases = {};
	};

	FUSSPhases(TArray<ELifecyclePhase> Phases) {
		mCalledPhases = Phases;
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ELifecyclePhase> mCalledPhases;
};

USTRUCT(BlueprintType)
struct FUSSCDOInformation {
	GENERATED_BODY()

	FUSSCDOInformation() {
		mItemStackSizeCDO.Add(EStackSize::SS_ONE, FUSSItemArray());
		mItemStackSizeCDO.Add(EStackSize::SS_SMALL, FUSSItemArray());
		mItemStackSizeCDO.Add(EStackSize::SS_MEDIUM, FUSSItemArray());
		mItemStackSizeCDO.Add(EStackSize::SS_BIG, FUSSItemArray());
		mItemStackSizeCDO.Add(EStackSize::SS_HUGE, FUSSItemArray());
		mItemStackSizeCDO.Add(EStackSize::SS_FLUID, FUSSItemArray());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UUSS_CDOHelperClass_Base>> mCDOHelperClasses = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EStackSize, FUSSItemArray> mItemStackSizeCDO;
};


/**
 * 
 */
UINTERFACE()
class UNIVERSALSWATCHSLOTS_API UUSSContentCDOHelperInterface : public UInterface
{
	GENERATED_BODY()
	
};

/**
 *
 */
class UNIVERSALSWATCHSLOTS_API IUSSContentCDOHelperInterface {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "KMods|ContentCDOHelper Interface")
	FUSSCDOInformation GetCDOInformationFromPhase(ELifecyclePhase Phase, bool& HasPhase);
};