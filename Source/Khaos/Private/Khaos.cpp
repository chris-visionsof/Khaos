// Fill out your copyright notice in the Description page of Project Settings.

#include "Khaos.h"
#include "Modules/ModuleManager.h"

#define GAME_NAME "Khaos"

FKhaosModule FKhaosModule::Get()
{
	static const FName ModuleName = GAME_NAME;
	return FModuleManager::LoadModuleChecked<FKhaosModule>(ModuleName);
}

IMPLEMENT_PRIMARY_GAME_MODULE(FKhaosModule, Khaos, GAME_NAME);
