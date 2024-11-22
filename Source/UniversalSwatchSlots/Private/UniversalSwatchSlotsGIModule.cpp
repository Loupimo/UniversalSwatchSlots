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

	/*if (NewClass)
	{
		UObject* tCDO = NewClass->GetDefaultObject();

		if (tCDO)
		{
			// Modify CDO properties of the newly generated class
			UUSSSwatchDesc* CDO = Cast<UUSSSwatchDesc>(tCDO);

			// Create an instance to return
			UUSSSwatchDesc* InstClass = NewObject<UUSSSwatchDesc>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *GenName)), RF_MarkAsRootSet | RF_Public);

			this->GeneratedClasses.Add(NewClass);

			return InstClass;
		}
	}*/

	return nullptr;
}


UUSSSwatchRecipe* UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchRecipe(int32 UniqueID)
{
	// Create a dynamic derivated class
	FString GenName = FString::Printf(TEXT("Gen_USS_SwatchRecipe_%d"), UniqueID);
	UClass* NewClass = GenerateDynamicClass(UUSSSwatchRecipe::StaticClass(), FName(*GenName));
	

	this->SwatchRecipeArray.Add(UniqueID, NewClass);
	
	/*
	if (NewClass)
	{
		UObject* tempCDO = NewClass->GetDefaultObject();

		if (tempCDO)
		{
			// Modify CDO properties of the newly generated class
			UUSSSwatchRecipe* CDO = Cast<UUSSSwatchRecipe>(tempCDO);


			// Create an instance to return
			UUSSSwatchRecipe* InstClass = NewObject<UUSSSwatchRecipe>(GetTransientPackage(), NewClass, FName(*FString::Printf(TEXT("Inst_%s"), *GenName)), RF_MarkAsRootSet | RF_Public);

			this->GeneratedClasses.Add(NewClass);
			return InstClass;
		}
	}*/

	return nullptr;
}


void UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchClasses()
{
	int32 startID = 28; // SF slots
	int32 maxSlots = 255; // Can't go further as SF is using uint8 to update its swatch colors. 255 is the ID of the custom swatch

	if (this->IsUsingMoreSwatchSlots)
	{	// We need to add the fixed slots of MSS

		startID += 20;
	}

	for (int32 i = startID; i < maxSlots; i++)
	{
		this->GenerateDynamicSwatchDescriptor(i);
		this->GenerateDynamicSwatchRecipe(i);
		//this->SwatchDescriptorArray.Add(i, this->GenerateDynamicSwatchDescriptor(i));
		//this->SwatchRecipeArray.Add(i, this->GenerateDynamicSwatchRecipe(i));
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