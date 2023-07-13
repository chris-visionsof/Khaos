// Fill out your copyright notice in the Description page of Project Settings.


#include "Khaos/Public/Actors/KhaosActionableActor.h"

#include "Khaos/Public/KhaosDefinitions.h"

AKhaosActionableActor::AKhaosActionableActor()
{
	Tags.Add(ActorTags::Actionable);
}

void AKhaosActionableActor::PerformAction()
{
	OnPerformAction_BP();
}

