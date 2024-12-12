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
	  * Safety wrapper around FClassGenerator::GenerateSimpleClass. Only for stuff in the UniversalSwatchSlots package.
	  * 
	  * @param	PackageName		The package name to store the class to.
	  * @param	Name			The name to give to class.
	  * @param	ParentClass		The parent class of this class.
	  * 
	  * @return If existing the matching class, otherwise the newly created class.
	  * 
	  */
	UFUNCTION(BlueprintCallable)
	static TSubclassOf<UObject> FindOrCreateClass(FString PackageName, FString Name, UClass* ParentClass);

	/**
	  * generate a texture representing a swatch icon based on the given colors.
	  *
	  * @param	PrimaryColor		The primary color (top left) to use.
	  * @param	SecondaryColor		The secondary color (bottom right) to use.
	  *
	  * @return The geenrated texture.
	  *
	  */
	UTexture2D* GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor);
};
