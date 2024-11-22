// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsGIModule.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlotsGI, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlotsGI)

UUSSSwatchDesc* UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchDescriptor(int32 UniqueID)
{
	FString GenName = FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), UniqueID);

	// Create a dynamic derivated class
	UClass* NewClass = GenerateDynamicClass(UUSSSwatchDesc::StaticClass(), FName(*GenName));
	
	this->SwatchDescriptorArray.Add(UniqueID, NewClass);

	return nullptr;
}


UUSSSwatchRecipe* UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchRecipe(int32 UniqueID)
{
	// Create a dynamic derivated class
	FString GenName = FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), UniqueID);
	UClass* NewClass = GenerateDynamicClass(UUSSSwatchRecipe::StaticClass(), FName(*GenName));
	
	this->SwatchRecipeArray.Add(UniqueID, NewClass);
	
	return nullptr;
}


void UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchClasses()
{
	int32 startID = 28; // SF slots. Start after SF slots no matter what because if we add MSS after this mod it can cause crash due to the first swatch classe not being created
	int32 maxSlots = 255; // Can't go further as SF is using uint8 to update its swatch colors. 255 is the ID of the custom swatch

	for (int32 i = startID; i < maxSlots; i++)
	{
		this->GenerateDynamicSwatchDescriptor(i);
		this->GenerateDynamicSwatchRecipe(i);
	}
}


UClass* UUniversalSwatchSlotsGIModule::GenerateDynamicClass(UClass* TemplateClass, FName GeneratedClassName)
{
	if (!TemplateClass || !TemplateClass->IsValidLowLevelFast())
	{
		return nullptr;
	}

	UClass* NewClass = NewObject<UClass>(GetTransientPackage(), GeneratedClassName, RF_Public | RF_Transient);

	if (NewClass)
	{
		// Setting UCLASS properties
		NewClass->PurgeClass(false);
		NewClass->ClassFlags |= CLASS_Transient;
		NewClass->PropertyLink = TemplateClass->PropertyLink;

		// Setting class constructor
		NewClass->ClassConstructor = TemplateClass->ClassConstructor;
		NewClass->ClassWithin = TemplateClass->ClassWithin;

		// Setting parent class
		NewClass->SetSuperStruct(TemplateClass);

		// Compile class
		NewClass->StaticLink(true);
		NewClass->Bind();
	}

	return NewClass;
}