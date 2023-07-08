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

void APCPlayerPawn::OnPlayerXMovementAction(bool bForward)
{
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), (bForward ? KeyboardInputMovementValue : (KeyboardInputMovementValue * -1)));
	}
}

void APCPlayerPawn::OnPlayerYMovementAction(bool bRight)
{
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorRightVector(), (bRight ? KeyboardInputMovementValue : (KeyboardInputMovementValue * -1)));
	}
}

void APCPlayerPawn::SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent)
{
	Super::SetupEnhancedPlayerInput(EnhancedInputSubsystem, PlayerInputComponent);

	EnhancedInputSubsystem->AddMappingContext(PCInputConfig->PCMappingContext, 1);
 
	PlayerInputComponent->BindAction(PCInputConfig->LookAndMotion.Get(), ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerLookAndMotionAction);
	PlayerInputComponent->BindAction(PCInputConfig->MoveForward, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerXMovementAction, true);
	PlayerInputComponent->BindAction(PCInputConfig->MoveBackward, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerXMovementAction, false);
	PlayerInputComponent->BindAction(PCInputConfig->StrifeRight, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerYMovementAction, true);
	PlayerInputComponent->BindAction(PCInputConfig->StrifeLeft, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerYMovementAction, false);
}


