#include "USSPaintModeWidget.h"

#include "UniversalSwatchSlots.h" // LogUSS

#include "Equipment/FGBuildGun.h"
#include "FGBuildGunModeDescriptor.h"
#include "Input/FGEnhancedInputComponent.h"

#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	// Tag declared in Config/Tags/GameplayTags.ini and mapped to IA_ModeSelect in DefaultInput.ini.
	static const FName GModeSelectInputTag(TEXT("Input.Equipment.BuildGun.ModeSelect"));
}

void UUSSPaintModeWidget::Setup(AFGBuildGun* InBuildGun, UFGEnhancedInputComponent* InInputComponent)
{
	BuildGun = InBuildGun;
	BoundInputComponent = InInputComponent;

	if (BuildGun)
	{
		BuildGun->mOnBuildGunModeChanged.AddDynamic(this, &UUSSPaintModeWidget::HandleBuildGunModeChanged);
	}

	if (BoundInputComponent)
	{
		const FGameplayTag tag = FGameplayTag::RequestGameplayTag(GModeSelectInputTag);
		// Bind on the same action the build gun uses. We get Started (press) and Completed
		// (release) reliably here, which the gun's own handler does not provide in Paint.
		BoundInputComponent->BindActionByTag(tag, ETriggerEvent::Started, this, &UUSSPaintModeWidget::HandleModeSelectStarted);
		BoundInputComponent->BindActionByTag(tag, ETriggerEvent::Completed, this, &UUSSPaintModeWidget::HandleModeSelectCompleted);
	}

	OnSetup();
	OnModeChanged(GetCurrentMode());
}

void UUSSPaintModeWidget::Teardown()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(HoldTimerHandle);
	}

	if (BuildGun)
	{
		BuildGun->mOnBuildGunModeChanged.RemoveDynamic(this, &UUSSPaintModeWidget::HandleBuildGunModeChanged);
	}

	if (BoundInputComponent)
	{
		// Remove only the bindings we added, leaving the gun's own bindings intact.
		BoundInputComponent->ClearBindingsForObject(this);
		BoundInputComponent = nullptr;
	}

	BuildGun = nullptr;
	bWheelOpen = false;
}

void UUSSPaintModeWidget::NativeDestruct()
{
	Teardown(); // safety net in case the widget is torn down without HideIndicator
	Super::NativeDestruct();
}

TArray<TSubclassOf<UFGBuildGunModeDescriptor>> UUSSPaintModeWidget::GetPaintModes() const
{
	TArray<TSubclassOf<UFGBuildGunModeDescriptor>> modes;
	if (BuildGun)
	{
		if (UFGBuildGunState* state = BuildGun->GetCurrentState())
		{
			state->GetSupportedBuildModes(modes);
		}
	}
	return modes;
}

TSubclassOf<UFGBuildGunModeDescriptor> UUSSPaintModeWidget::GetCurrentMode() const
{
	return BuildGun ? BuildGun->GetCurrentBuildGunMode() : nullptr;
}

void UUSSPaintModeWidget::SetPaintMode(TSubclassOf<UFGBuildGunModeDescriptor> Mode)
{
	if (BuildGun && Mode)
	{
		BuildGun->SetCurrentBuildGunMode(Mode);
	}
}

void UUSSPaintModeWidget::CyclePaintMode(int32 Delta)
{
	const TArray<TSubclassOf<UFGBuildGunModeDescriptor>> modes = GetPaintModes();
	if (modes.Num() <= 1 || !BuildGun)
	{
		return;
	}

	const int32 current = modes.IndexOfByKey(BuildGun->GetCurrentBuildGunMode());
	const int32 base = (current == INDEX_NONE) ? 0 : current;
	const int32 next = ((base + Delta) % modes.Num() + modes.Num()) % modes.Num();
	BuildGun->SetCurrentBuildGunMode(modes[next]);
}

void UUSSPaintModeWidget::HandleModeSelectStarted()
{
	bWheelOpen = false;

	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().SetTimer(
			HoldTimerHandle, this, &UUSSPaintModeWidget::OnHoldThresholdReached,
			FMath::Max(WheelHoldThreshold, 0.01f), false);
	}
}

void UUSSPaintModeWidget::OnHoldThresholdReached()
{
	bWheelOpen = true;
	OnWheelShow();
}

void UUSSPaintModeWidget::HandleModeSelectCompleted()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(HoldTimerHandle);
	}

	if (bWheelOpen)
	{
		bWheelOpen = false;
		OnWheelCommit(); // wheel was open -> apply the highlighted option (Blueprint)
	}
	else
	{
		CyclePaintMode(1); // quick tap -> just toggle
	}
}

void UUSSPaintModeWidget::HandleBuildGunModeChanged(TSubclassOf<UFGBuildGunModeDescriptor> NewMode)
{
	OnModeChanged(NewMode);
}
