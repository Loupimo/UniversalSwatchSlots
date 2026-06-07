

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
