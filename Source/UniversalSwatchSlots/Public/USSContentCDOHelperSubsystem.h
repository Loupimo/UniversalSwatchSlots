// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Module/ModModule.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "USSContentCDOHelperInterface.h"
#include "USSCustomizerInterface.h"
#include "USSContentCDOHelperSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSContentCDOHelperSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	static UUSSContentCDOHelperSubsystem* Get(UObject* Context);

	
	UFUNCTION(BlueprintCallable, Category = "KMods")
	void BeginCDOForModule(UModModule* Module, ELifecyclePhase Phase);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	bool WasCDOForModuleCalled(UModModule* Module, ELifecyclePhase Phase) const;

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void ResetCDOCallFromModule(UModModule* Module);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void DoCDOFromInfo(FUSSCDOInformation Info);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void CallCDOHelper(TSubclassOf<UUSS_CDOHelperClass_Base> CDOHelperClass, bool IgnoreCallCheck = false);

	UFUNCTION(BlueprintPure, Category = "KMods", meta = (DeterminesOutputType = "Class"))
	UObject* GetAndStoreDefaultObject(UClass* Class);

	UFUNCTION(BlueprintPure, Category = "KMods", meta = (DeterminesOutputType = "Class", DefaultToSelf = "Context"))
	static UObject* GetAndStoreCDO(UClass* Class, UObject* Context);

	template<typename T>
	T* GetAndStoreDefaultObject_Native(UClass* Class);

	template<typename T>
	static T* GetAndStoreDefaultObject_Native(UClass* Class, UObject* Context);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void StoreClass(UClass* Class);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void StoreObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void RemoveClass(UClass* Class);

	UFUNCTION(BlueprintCallable, Category = "KMods")
	void RemoveObject(UObject* Object);

	UFUNCTION(BlueprintPure)
	TArray<UModModule*> GetCalledModules() const;

private:
	bool Initialized = false;

	UPROPERTY()
	TArray<UModModule*> mModulesToCall;

	UPROPERTY()
	TMap<UModModule*, FUSSPhases> mCDOCalled;

	UPROPERTY()
	TArray<UObject*> mCalledObjects;

	UPROPERTY()
	TArray<UClass*> mCalledClasses;
};

template<typename T>
T* UUSSContentCDOHelperSubsystem::GetAndStoreDefaultObject_Native(UClass* Class) {
	if (IsValid(Class)) {
		mCalledClasses.AddUnique(Class);
		mCalledObjects.AddUnique(Class->GetDefaultObject());
		return Cast<T>(Class->GetDefaultObject());
	}
	return nullptr;
}

template<typename T>
T* UUSSContentCDOHelperSubsystem::GetAndStoreDefaultObject_Native(UClass* Class, UObject* Context) {
	if (UUSSContentCDOHelperSubsystem* Sub = Get(Context)) {
		return Sub->GetAndStoreDefaultObject_Native<T>(Class);
	}
	return nullptr;
}
