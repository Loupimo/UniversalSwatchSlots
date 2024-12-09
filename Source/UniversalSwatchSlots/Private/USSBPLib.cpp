// Fill out your copyright notice in the Description page of Project Settings.


#include "USSBPLib.h"
#include "Reflection/ClassGenerator.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniversalSwatchSlotsLib, Log, All)

DEFINE_LOG_CATEGORY(LogUniversalSwatchSlotsLib)


TSubclassOf<UObject> UUSSBPLib::FindOrCreateClass(FString Name, UClass* ParentClass)
{
	// Safety checks before running the generator
	auto found = FindObject<UClass>(FindPackage(nullptr, TEXT("/UniversalSwatchSlots/")), *Name, false);
	if (Name == "") {
		UE_LOG(LogUniversalSwatchSlotsLib, Error, TEXT("Name was empty, can't create class"));
		return nullptr;
	}
	if (found) {
		UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Class already existed so returning that instead of creating new"));
		return found;
	}
	auto found_C = FindObject<UClass>(FindPackage(nullptr, TEXT("/UniversalSwatchSlots/")), *Name.Append("_C"), false);
	if (found_C) {
		UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Class already existed with _C suffix so returning that instead of creating new"));
		return found_C;
	}
	UE_LOG(LogUniversalSwatchSlotsLib, Verbose, TEXT("Creating new class : %s"), *Name);

	return FClassGenerator::GenerateSimpleClass(TEXT("/UniversalSwatchSlots/"), *Name, ParentClass);
}