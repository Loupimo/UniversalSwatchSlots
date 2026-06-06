// Copyright Epic Games, Inc. All Rights Reserved.

#include "UniversalSwatchSlots.h"
#include "Equipment/FGBuildGunPaint.h"
#include "Patching/NativeHookManager.h"
#include "USSModeDescriptor.h"

#define LOCTEXT_NAMESPACE "FUniversalSwatchesModule"

DECLARE_LOG_CATEGORY_EXTERN(LogUSS, Log, All)
DEFINE_LOG_CATEGORY(LogUSS)

void FUniversalSwatchSlotsModule::StartupModule()
{
	/*if (!WITH_EDITOR)
	{
		UFGBuildGunStatePaint* bgp = GetMutableDefault<UFGBuildGunStatePaint>();
		SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::GetSupportedBuildModes, bgp, [this](auto& scope, const UFGBuildGunState* BuildGunPaint, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildModes)
			{
				out_buildModes.Add(UUSSModeDescriptor::StaticClass());
				UE_LOG(LogUSS, Warning, TEXT("Modes count = %d"), out_buildModes.Num());
			});
	}*/
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUniversalSwatchSlotsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUniversalSwatchSlotsModule, UniversalSwatchSlots)