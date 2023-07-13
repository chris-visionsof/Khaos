// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KhaosActor.h"
#include "KhaosActionableActor.generated.h"

UCLASS(Abstract)
class KHAOS_API AKhaosActionableActor : public AKhaosActor
{
	GENERATED_BODY()

public:
	AKhaosActionableActor();

	void PerformAction();

protected:
	UFUNCTION(BlueprintImplementableEvent, DisplayName="On Perform Action", Category="Interactivity")
	void OnPerformAction_BP();
};
