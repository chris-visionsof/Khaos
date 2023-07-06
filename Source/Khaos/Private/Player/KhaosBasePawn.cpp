// Fill out your copyright notice in the Description page of Project Settings.


#include "KhaosBasePawn.h"


// Sets default values
AKhaosBasePawn::AKhaosBasePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called to bind functionality to input
void AKhaosBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

