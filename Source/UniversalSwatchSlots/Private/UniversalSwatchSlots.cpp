// Copyright Epic Games, Inc. All Rights Reserved.

#include "UniversalSwatchSlots.h"
#include "USSBuildGunPaintMode.h"

#define LOCTEXT_NAMESPACE "FUniversalSwatchesModule"

DEFINE_LOG_CATEGORY(LogUSS);

void FUniversalSwatchSlotsModule::StartupModule()
{
	// Game only: install the build gun "Paint mode" hooks (Default / Blueprint toggle + HUD indicator).
	if (!WITH_EDITOR)
	{
		FUSSBuildGunPaintMode::RegisterHooks();
	}
}

void FUniversalSwatchSlotsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUniversalSwatchSlotsModule, UniversalSwatchSlots)
