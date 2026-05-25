// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsGIModule.h"
#include "Reflection/ClassGenerator.h"
#include "USSBPLib.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlotsGI, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlotsGI)

void UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchDescriptor(int32 UniqueID)
{
	FString GenName = FString::Printf(TEXT("Gen_USS_SwatchDesc_%d"), UniqueID);

	if (this->SwatchDescriptorArray.Contains(UniqueID))
	{	// A swatch descriptor class already exist at this ID, reuse it.

		return;
	}

	UE_LOG(LogUniversalSwatchSlotsGI, Display, TEXT("Creating new class : %s"), *GenName);

	FUSSSwatchDescGenInfo newSwatchDesc;

	// Create the swatch descriptor class but no instance. Instance will be created by the USSSubsytem at world loading.
	newSwatchDesc.SwatchClass = UUSSBPLib::CreateClass(this->PackageName, GenName + FString("_Class"), UUSSSwatchDesc::StaticClass());
	newSwatchDesc.SwatchCDO = Cast<UUSSSwatchDesc>(newSwatchDesc.SwatchClass->GetDefaultObject());
	newSwatchDesc.SwatchInst = nullptr;

	this->SwatchDescriptorArray.Add(UniqueID, newSwatchDesc);

	// 1.2.2 backward compatibility
	UClass* NewClass = (UClass*)UUSSBPLib::CreateClass(this->PackageName, GenName, UUSSSwatchDesc::StaticClass());
	this->tmpSwatchDescriptorArray.Add(NewClass);

	// 1.1.0 - 1.2.1 backward compatibility
	NewClass = (UClass*)UUSSBPLib::CreateClass(this->PackageName, GenName.Append("_C"), UUSSSwatchDesc::StaticClass());
	this->tmpSwatchDescriptorArray.Add(NewClass);

	/*

	// Create a dynamic derivated class
	UClass* NewClass = (UClass * )UUSSBPLib::FindOrCreateClass(this->PackageName, GenName, UUSSSwatchDesc::StaticClass());
	this->SwatchDescriptorArray.Add(UniqueID, NewClass);*/

	// This is needed until 1.0.5 as people may have this wrong package name for their swatches
	/*NewClass = (UClass*)UUSSBPLib::FindOrCreateClass(FString("/UniversalSwatchSlots/"), GenName, UUSSSwatchDesc::StaticClass());
	this->tmpSwatchDescriptorArray.Add(NewClass);
	NewClass = (UClass*)GenerateDynamicClass(UUSSSwatchDesc::StaticClass(), FName(*GenName));
	this->tmpSwatchDescriptorArray.Add(NewClass);*/
}


void UUniversalSwatchSlotsGIModule::GenerateDynamicSwatchClasses()
{
	int32 startID = 28; // SF slots. Start after SF slots no matter what because if we add MSS after this mod it can cause crash due to the first swatch classe not being created
	int32 maxSlots = 255; // Can't go further as SF is using uint8 to update its swatch colors. 255 is the ID of the custom swatch

	for (int32 i = startID; i < maxSlots; i++)
	{
		this->GenerateDynamicSwatchDescriptor(i);
		// Skip recipe generation to prevent ReliableBufferOverflow replication bloat; descriptors alone are sufficient for save compatibility / building patching.
	}
}
