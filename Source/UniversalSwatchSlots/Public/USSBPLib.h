// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
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
	  * Generate a texture representing a swatch icon based on the given colors.
	  *
	  * @param	PrimaryColor		The primary color (top left) to use.
	  * @param	SecondaryColor		The secondary color (bottom right) to use.
	  *
	  * @return The generated texture.
	  *
	  */
	UFUNCTION(BlueprintCallable)
	static UTexture2D* GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor);

	/**
	  * Convert the given hexadecimal string to a FLinearColor for SF.
	  *
	  * @param	HexCode		The hexadecimal color (#RRGGBBAA, RRGGBBAA, #RRGGBB, RRGGBB).
	  *
	  * @return Matching linear color.
	  *
	  */
	UFUNCTION(BlueprintCallable)
	static FLinearColor HexToLinearColor(FString HexCode);

	/**
	 * Build the swatch name based on the given parameter.
	 *
	 *
	 * @param	SwatchDisplayName		The swatch display name.
	 * @param	SwatchClassAcr			The swatch class acronym.
	 * @param	SwatchID				The swatch ID. It should match the occurrence number of the swatch display name you are trying to find (starting at index 0).
	 *
	 * @return The matching generated name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Swatch")
	static FString BuildSwatchGenName(FString SwatchDisplayName, FString SwatchClassAcr, int32 SwatchID);

};
