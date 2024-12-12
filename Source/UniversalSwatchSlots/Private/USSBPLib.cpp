// Fill out your copyright notice in the Description page of Project Settings.


#include "USSBPLib.h"
#include "Reflection/ClassGenerator.h"
#include "RenderUtils.h"
#include "Math/Vector2D.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlotsLib, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlotsLib)


TSubclassOf<UObject> UUSSBPLib::FindOrCreateClass(FString PackageName, FString Name, UClass* ParentClass)
{
	// Safety checks before running the generator
	auto found = FindObject<UClass>(FindPackage(nullptr, *PackageName), *Name, false);
	if (Name == "") {
		UE_LOG(LogUniversalSwatchSlotsLib, Error, TEXT("Name was empty, can't create class"));
		return nullptr;
	}
	if (found) {
		UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Class already existed so returning that instead of creating new"));
		return found;
	}
	auto found_C = FindObject<UClass>(FindPackage(nullptr, *PackageName), *Name.Append("_C"), false);
	if (found_C) {
		UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Class already existed with _C suffix so returning that instead of creating new"));
		return found_C;
	}
	UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Creating new class : %s"), *Name);

	return FClassGenerator::GenerateSimpleClass(*PackageName, *Name, ParentClass);
}

UTexture2D* UUSSBPLib::GenerateSwatchIcon(FLinearColor PrimaryColor, FLinearColor SecondaryColor)
{
	int32 Width = 128;
	int32 ArcThickness = 15;
	FColor ArcColor = FColor(192, 192, 192, 255);
	FColor prim = PrimaryColor.ToFColor(true);
	FColor sec = SecondaryColor.ToFColor(true);

	if (Width <= 0 || ArcThickness <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid texture dimensions or arc thickness."));
		return nullptr;
	}

	// Check that the width is a power a 2 to ensure optimal compatibility
	if ((Width & (Width - 1)) != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Width is not a power of 2. The texture might not behave optimally in some scenarios."));
	}

	// Circle radius and center
	int32 Radius = Width / 2;
	FVector2D Center(Radius, Radius);

	// Create a transient texture
	UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Width, PF_B8G8R8A8);
	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create transient texture."));
		return nullptr;
	}

	// Prevent garbage collector to delete the texture
	NewTexture->AddToRoot();

	// Initialiser texture data
	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* Pixels = static_cast<FColor*>(Data);

	// Fill the texture
	for (int32 Y = 0; Y < Width; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			FVector2D Pixel(X, Y);
			float DistanceToCenter = FVector2D::Distance(Pixel, Center);

			if (DistanceToCenter > Radius - ArcThickness && DistanceToCenter <= Radius)
			{
				// Pixel dans l'arceau
				Pixels[Y * Width + X] = ArcColor;
			}
			else if (DistanceToCenter <= Radius - ArcThickness)
			{
				// Pixel inside the circle
				if (X >= Width - Y - 1) // Inverted diagonale 
				{
					Pixels[Y * Width + X] = sec;
				}
				else
				{
					Pixels[Y * Width + X] = prim;
				}
			}
			else
			{
				// Pixel en dehors du cercle
				Pixels[Y * Width + X] = FColor::Transparent; // Transparent color
			}
		}
	}

	// Unlock data and update texture
	Mip.BulkData.Unlock();
	NewTexture->UpdateResource();

	return NewTexture;
}