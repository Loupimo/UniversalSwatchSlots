// Copyright Epic Games, Inc. All Rights Reserved.

#include "UniversalSwatchSlots.h"
#include "Equipment/FGBuildGunPaint.h"
#include "Equipment/FGBuildGun.h"
#include "Patching/NativeHookManager.h"
#include "USSModeDescriptor.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

#define LOCTEXT_NAMESPACE "FUniversalSwatchesModule"

// Path to the in-game paint-mode indicator widget (created in the mod's Content in UE5).
#define USS_PAINT_MODE_WIDGET_PATH TEXT("/UniversalSwatchSlots/UI/Widget_USSPaintMode.Widget_USSPaintMode_C")

DECLARE_LOG_CATEGORY_EXTERN(LogUSS, Log, All)
DEFINE_LOG_CATEGORY(LogUSS)

// Current paint-mode indicator instance (local player only). Kept alive by the viewport.
static TWeakObjectPtr<UUserWidget> GPaintModeWidget;

void FUniversalSwatchSlotsModule::StartupModule()
{
	if (!WITH_EDITOR)
	{
		UFGBuildGunStatePaint* bgp = GetMutableDefault<UFGBuildGunStatePaint>();

		// (a) Expose two build modes (Default + Blueprint) while in the Paint state,
		//     so the build gun shows a "Default / Blueprint" roller like Dismantle.
		SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::GetSupportedBuildModes, bgp, [](auto& scope, const UFGBuildGunState* state, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildModes)
			{
				// Run the game's implementation first so we append instead of getting overwritten.
				scope(state, out_buildModes);

				// This stub is shared by every state, so only add our modes for Paint.
				if (state && state->IsA<UFGBuildGunStatePaint>())
				{
					out_buildModes.AddUnique(UUSSPaintModeDefault::StaticClass());
					out_buildModes.AddUnique(UUSSModeDescriptor::StaticClass());
					UE_LOG(LogUSS, Warning, TEXT("Paint GetSupportedBuildModes -> %d modes"), out_buildModes.Num());
				}
				else
				{
					UE_LOG(LogUSS, Warning, TEXT("Other GetSupportedBuildModes -> %d modes"), out_buildModes.Num());
				}
			});

		// (b) Start in "Default" when entering Paint so the selector has a valid current mode.
		SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStatePaint::GetInitialBuildGunMode, bgp, [](auto& scope, const UFGBuildGunState* state)
			{
				if (state && state->IsA<UFGBuildGunStatePaint>())
				{
					scope.Override(TSubclassOf<UFGBuildGunModeDescriptor>(UUSSPaintModeDefault::StaticClass()));
				}
				// Other states: leave untouched, the original is auto-forwarded.
			});

		SUBSCRIBE_METHOD(AFGBuildGun::SetCurrentBuildGunMode, [](auto& scope, AFGBuildGun* self, TSubclassOf<UFGBuildGunModeDescriptor> mode)
			{
				UE_LOG(LogUSS, Warning, TEXT("SetCurrentBuildGunMode: %s"), *GetNameSafe(mode));

				scope(self, mode);
			});

		// Vanilla cycles build modes on the mode-select key (R) for Build/Dismantle but
		// NOT in the Paint state, even though the input reaches the gun and the mode list
		// is correct in Paint (both confirmed via logs). So we drive the toggle ourselves
		// on press, only while in Paint (other states keep their vanilla behavior).
		SUBSCRIBE_METHOD(AFGBuildGun::OnModeSelectPressed, [](auto& scope, AFGBuildGun* self)
			{
				scope(self); // let the gun run its normal press logic first

				UFGBuildGunState* state = self ? self->GetCurrentState() : nullptr;
				if (state && state->IsA<UFGBuildGunStatePaint>())
				{
					TArray<TSubclassOf<UFGBuildGunModeDescriptor>> modes;
					state->GetSupportedBuildModes(modes);
					if (modes.Num() > 1)
					{
						const int32 idx = modes.IndexOfByKey(self->GetCurrentBuildGunMode());
						const int32 nextIdx = (idx == INDEX_NONE) ? 0 : (idx + 1) % modes.Num();
						UE_LOG(LogUSS, Warning, TEXT("Paint cycle: %d -> %d (%s)"), idx, nextIdx, *GetNameSafe(modes[nextIdx]));
						self->SetCurrentBuildGunMode(modes[nextIdx]);
					}
				}
			});

		// --- Phase 1: persistent paint-mode indicator widget ---------------
		// The HUD has no mode widget for the Paint state, so we inject our own
		// mod-content UserWidget into the viewport while the Paint state is active.
		SUBSCRIBE_METHOD(UFGBuildGunStatePaint::BeginState, [](auto& scope, UFGBuildGunState* state)
			{
				scope(state); // run the game's BeginState first

				if (!state || !state->IsA<UFGBuildGunStatePaint>())
				{
					return;
				}

				AFGBuildGun* gun = state->GetBuildGun();
				APawn* pawn = gun ? gun->GetInstigator() : nullptr;
				if (!pawn || !pawn->IsLocallyControlled())
				{
					return; // only the local player gets the HUD widget
				}

				APlayerController* pc = Cast<APlayerController>(pawn->GetController());
				if (!pc)
				{
					return;
				}

				UClass* widgetClass = LoadClass<UUserWidget>(nullptr, USS_PAINT_MODE_WIDGET_PATH);
				if (!widgetClass)
				{
					UE_LOG(LogUSS, Warning, TEXT("Paint mode widget class not found at %s"), USS_PAINT_MODE_WIDGET_PATH);
					return;
				}

				if (GPaintModeWidget.IsValid())
				{
					GPaintModeWidget->RemoveFromParent();
					GPaintModeWidget.Reset();
				}

				if (UUserWidget* widget = CreateWidget<UUserWidget>(pc, widgetClass))
				{
					widget->AddToViewport();
					GPaintModeWidget = widget;
				}
			});

		SUBSCRIBE_METHOD(UFGBuildGunStatePaint::EndState, [](auto& scope, UFGBuildGunState* state)
			{
				scope(state);

				if (state && state->IsA<UFGBuildGunStatePaint>() && GPaintModeWidget.IsValid())
				{
					GPaintModeWidget->RemoveFromParent();
					GPaintModeWidget.Reset();
				}
			});
		// -------------------------------------------------------------------
	}
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUniversalSwatchSlotsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUniversalSwatchSlotsModule, UniversalSwatchSlots)