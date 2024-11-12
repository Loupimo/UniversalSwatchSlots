// Fill out your copyright notice in the Description page of Project Settings.


#include "USSContentCDOHelperSubsystem.h"

#include "Kismet/KismetSystemLibrary.h"
#include "FGGameState.h"
#include "Module/WorldModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ContentCDOHelperSubsystem, Log, All)

DEFINE_LOG_CATEGORY(ContentCDOHelperSubsystem)


void UUSSContentCDOHelperSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
#endif
}

void UUSSContentCDOHelperSubsystem::Deinitialize()
{
	Initialized = false;

	mModulesToCall.Empty();
	mCDOCalled.Empty();
	mCalledObjects.Empty();

	Super::Deinitialize();
}

UUSSContentCDOHelperSubsystem* UUSSContentCDOHelperSubsystem::Get(UObject* Context)
{
	if (IsValid(Context)) {
		return Context->GetWorld()->GetGameInstance()->GetSubsystem<UUSSContentCDOHelperSubsystem>();
	}
	return nullptr;
}

void UUSSContentCDOHelperSubsystem::BeginCDOForModule(UModModule* Module, ELifecyclePhase Phase)
{
	UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("BeginCDOForModule > Was Called - Mod: %s"), *Module->GetOwnerModReference().ToString());
	if (UKismetSystemLibrary::DoesImplementInterface(Module, UUSSContentCDOHelperInterface::StaticClass())) {
		if (!WasCDOForModuleCalled(Module, Phase)) {
			UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("Try to get information for mod: %s"), *Module->GetOwnerModReference().ToString());
			bool                HasPhase;
			FUSSCDOInformation Info = IUSSContentCDOHelperInterface::Execute_GetCDOInformationFromPhase(Module, Phase, HasPhase);
			if (HasPhase) {
				UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("BeginCDOForModule > HasPhase (Start CDO on Phase: %s) - Mod: %s"), *Module->LifecyclePhaseToString(Phase), *Module->GetOwnerModReference().ToString());
				DoCDOFromInfo(Info);
				if (!mCDOCalled.Contains(Module)) {
					mCDOCalled.Add(Module);
				}
				mCDOCalled[Module].mCalledPhases.Add(Phase);
			}
			else {
				UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("Scrip phase - Mod: %s"), *Module->GetOwnerModReference().ToString());
			}
		}
		else {
			UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("CDO Was Called for this phase: %s"), *Module->GetOwnerModReference().ToString());
		}
	}
}

bool UUSSContentCDOHelperSubsystem::WasCDOForModuleCalled(UModModule* Module, ELifecyclePhase Phase) const {
	if (Module) {
		if (mCDOCalled.Contains(Module)) {
			return mCDOCalled[Module].mCalledPhases.Contains(Phase);
		}
	}
	return false;
}

void UUSSContentCDOHelperSubsystem::ResetCDOCallFromModule(UModModule* Module) {
	if (Module) {
		if (mCDOCalled.Contains(Module)) {
			mCDOCalled[Module].mCalledPhases.Empty();
		}
	}
}

void UUSSContentCDOHelperSubsystem::DoCDOFromInfo(FUSSCDOInformation Info) {
	UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("DoCDOFromInfo"));

	for (TSubclassOf<UUSS_CDOHelperClass_Base> CDOHelper : Info.mCDOHelperClasses) {
		if (IsValid(CDOHelper)) {
			CallCDOHelper(CDOHelper);
		}
	}
}

void UUSSContentCDOHelperSubsystem::CallCDOHelper(TSubclassOf<UUSS_CDOHelperClass_Base> CDOHelperClass, bool IgnoreCallCheck) {
	if (IsValid(CDOHelperClass)) {
		if (UUSS_CDOHelperClass_Base* CDOHelper = GetAndStoreDefaultObject_Native<UUSS_CDOHelperClass_Base>(CDOHelperClass)) {
			if (CDOHelper->ExecuteAllowed() || IgnoreCallCheck) {
				CDOHelper->mSubsystem = this;
				CDOHelper->ModifyValues();
				CDOHelper->DoCDO();
				CDOHelper->ExecuteBlueprintCDO();
				UE_LOG(ContentCDOHelperSubsystem, Log, TEXT("Do CDO Call for Helper: %s"), *CDOHelper->GetName());
			}
		}
	}
}

UObject* UUSSContentCDOHelperSubsystem::GetAndStoreDefaultObject(UClass* Class) {
	if (IsValid(Class)) {
		mCalledClasses.AddUnique(Class);
		mCalledObjects.AddUnique(Class->GetDefaultObject());
		return Class->GetDefaultObject();
	}
	return nullptr;
}

UObject* UUSSContentCDOHelperSubsystem::GetAndStoreCDO(UClass* Class, UObject* Context) {
	if (UUSSContentCDOHelperSubsystem* Sub = Get(Context)) {
		return Sub->GetAndStoreDefaultObject(Class);
	}
	return nullptr;
}


void UUSSContentCDOHelperSubsystem::StoreClass(UClass* Class) {
	mCalledClasses.AddUnique(Class);
}

void UUSSContentCDOHelperSubsystem::StoreObject(UObject* Object) {
	mCalledObjects.AddUnique(Object);
}

void UUSSContentCDOHelperSubsystem::RemoveClass(UClass* Class) {
	mCalledClasses.Remove(Class);
}

void UUSSContentCDOHelperSubsystem::RemoveObject(UObject* Object) {
	mCalledObjects.Remove(Object);
}

TArray<UModModule*> UUSSContentCDOHelperSubsystem::GetCalledModules() const {
	TArray<UModModule*> Modules;
	mCDOCalled.GenerateKeyArray(Modules);
	return Modules;
}