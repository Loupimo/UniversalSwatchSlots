

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
