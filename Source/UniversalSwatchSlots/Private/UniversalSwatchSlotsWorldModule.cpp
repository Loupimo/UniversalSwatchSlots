// Fill out your copyright notice in the Description page of Project Settings.


#include "UniversalSwatchSlotsWorldModule.h"


void UUniversalSwatchSlotsWorldModule::GenerateSwatchesFromPalette(FUSSPalette Palette)
{
	this->USSSubsystem->GeneratePalette(Palette);
}


void UUniversalSwatchSlotsWorldModule::InitUSSGameWorldModule(AUniversalSwatchSlotsSubsystem* Subsystem)
{
	this->USSSubsystem = Subsystem;
	this->USSSubsystem->IsUsingMSS = this->IsUsingMoreSwatchSlots;
	this->USSSubsystem->RetrieveFreeColorSlotID();
}