// Fill out your copyright notice in the Description page of Project Settings.


#include "PCPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "PCInputDataConfig.h"

APCPlayerPawn::APCPlayerPawn() : AKhaosBasePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
}

void APCPlayerPawn::OnPlayerLookAndMotionAction(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APCPlayerPawn::SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent)
{
	Super::SetupEnhancedPlayerInput(EnhancedInputSubsystem, PlayerInputComponent);

	EnhancedInputSubsystem->AddMappingContext(PCInputConfig->PCMappingContext, 1);
 
	PlayerInputComponent->BindAction(PCInputConfig->LookAndMotion.Get(), ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerLookAndMotionAction);
}


