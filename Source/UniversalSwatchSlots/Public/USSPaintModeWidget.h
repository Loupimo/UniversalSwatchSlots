#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TimerHandle.h"
#include "USSPaintModeWidget.generated.h"

class AFGBuildGun;
class UFGBuildGunModeDescriptor;
class UFGEnhancedInputComponent;

/**
 * Base class for the paint-mode HUD widget (indicator + radial wheel).
 *
 * The C++ side owns the input: it binds the build gun's "ModeSelect" action while in
 * the Paint state and turns it into high-level events for the Blueprint to react to:
 *   - quick tap        -> cycles to the next build mode (handled in C++)
 *   - hold past delay   -> OnWheelShow() (open the radial)
 *   - release with wheel-> OnWheelCommit() (apply the highlighted option)
 *
 * The Blueprint subclass only does the visuals: build the indicator and the wheel,
 * highlight the option under the cursor, and call SetPaintMode() on commit.
 *
 * Reparent your Widget_USSPaintMode blueprint to this class.
 */
UCLASS(Abstract, Blueprintable)
class UNIVERSALSWATCHSLOTS_API UUSSPaintModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called by the mod right after the widget is created for the local player. */
	void Setup(AFGBuildGun* InBuildGun, UFGEnhancedInputComponent* InInputComponent);

	/** Called by the mod when leaving the Paint state, before the widget is removed. */
	void Teardown();

	/** The build gun this widget is bound to. */
	UPROPERTY(BlueprintReadOnly, Category = "USS|PaintMode")
	TObjectPtr<AFGBuildGun> BuildGun;

	/** How long (seconds) the ModeSelect key must be held to open the wheel instead of cycling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USS|PaintMode")
	float WheelHoldThreshold = 0.18f;

	//~ Helpers for the Blueprint wheel / indicator ---------------------------------

	/** All build modes available in the paint state (Default, Blueprint, ...). */
	UFUNCTION(BlueprintPure, Category = "USS|PaintMode")
	TArray<TSubclassOf<UFGBuildGunModeDescriptor>> GetPaintModes() const;

	/** The currently selected build mode. */
	UFUNCTION(BlueprintPure, Category = "USS|PaintMode")
	TSubclassOf<UFGBuildGunModeDescriptor> GetCurrentMode() const;

	/** Apply a build mode. Call this from OnWheelCommit with the highlighted option. */
	UFUNCTION(BlueprintCallable, Category = "USS|PaintMode")
	void SetPaintMode(TSubclassOf<UFGBuildGunModeDescriptor> Mode);

	/** Cycle to the next (delta>0) / previous (delta<0) build mode. */
	UFUNCTION(BlueprintCallable, Category = "USS|PaintMode")
	void CyclePaintMode(int32 Delta = 1);

	//~ Blueprint visual events ------------------------------------------------------

	/** Called once after Setup. Initialise your visuals here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "USS|PaintMode")
	void OnSetup();

	/** The ModeSelect key was held past the threshold: open the radial wheel. */
	UFUNCTION(BlueprintImplementableEvent, Category = "USS|PaintMode")
	void OnWheelShow();

	/** The key was released while the wheel was open: apply the highlighted option and hide the wheel. */
	UFUNCTION(BlueprintImplementableEvent, Category = "USS|PaintMode")
	void OnWheelCommit();

	/** The current build mode changed: refresh the indicator. */
	UFUNCTION(BlueprintImplementableEvent, Category = "USS|PaintMode")
	void OnModeChanged(TSubclassOf<UFGBuildGunModeDescriptor> NewMode);

protected:
	virtual void NativeDestruct() override;

private:
	/** Bound to ETriggerEvent::Started of the ModeSelect action. */
	void HandleModeSelectStarted();

	/** Bound to ETriggerEvent::Completed of the ModeSelect action. */
	void HandleModeSelectCompleted();

	/** Fired by the hold timer when the key has been held long enough to open the wheel. */
	void OnHoldThresholdReached();

	/** Bound to the build gun's OnBuildGunModeChanged delegate. */
	UFUNCTION()
	void HandleBuildGunModeChanged(TSubclassOf<UFGBuildGunModeDescriptor> NewMode);

	UPROPERTY()
	TObjectPtr<UFGEnhancedInputComponent> BoundInputComponent;

	FTimerHandle HoldTimerHandle;

	/** True once the wheel has been opened during the current hold. */
	bool bWheelOpen = false;
};
