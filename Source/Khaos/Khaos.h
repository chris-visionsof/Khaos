// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FKhaosModule final : public IModuleInterface
{
public:
	virtual bool IsGameModule() const override { return true; };
	static FKhaosModule Get();
};
