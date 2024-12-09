// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "USSBPLib.generated.h"

/**
 * 
 */
UCLASS()
class UNIVERSALSWATCHSLOTS_API UUSSBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	  * Safety wrapper around FClassGenerator::GenerateSimpleClass. Only for stuff in the UniversalSwatchSlots package
	  * 
	  * @param	Name			The name to give to class.
	  * @param	ParentClass		The parent class of this class.
	  * 
	  * @return If existing the matching class, otherwise the newly created class.
	  * 
	  */
	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UObject> FindOrCreateClass(FString Name, UClass* ParentClass);
};
