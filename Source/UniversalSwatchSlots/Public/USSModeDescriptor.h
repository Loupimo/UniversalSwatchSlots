

#pragma once

#include "CoreMinimal.h"
#include "FGBuildGunModeDescriptor.h"
#include "USSModeDescriptor.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSModeDescriptor : public UFGBuildGunModeDescriptor
{
	GENERATED_BODY()

public:

	UUSSModeDescriptor()
	{
		mDisplayName = FText::FromString(TEXT("Blueprint"));
	}
};

/**
 * Default paint mode (single building). Paired with UUSSModeDescriptor so the
 * Paint state shows a "Default / Blueprint" roller like the Dismantle state.
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSPaintModeDefault : public UFGBuildGunModeDescriptor
{
	GENERATED_BODY()

public:

	UUSSPaintModeDefault()
	{
		mDisplayName = FText::FromString(TEXT("Default"));
	}
};

/**
 * "Same Swatch" paint mode: paints only the blueprint-plan elements that share the same
 * swatch as the aimed (or locked) element. See FUSSSameSwatchPaintMode.
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSPaintSameSwatchModeDescriptor : public UFGBuildGunModeDescriptor
{
	GENERATED_BODY()

public:

	UUSSPaintSameSwatchModeDescriptor()
	{
		mDisplayName = FText::FromString(TEXT("Same Swatch"));
	}
};
