// Fill out your copyright notice in the Description page of Project Settings.


#include "KhaosBasePawn.h"

#include "DefaultInputConfigData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Khaos/Public/KhaosDefinitions.h"

AKhaosBasePawn::AKhaosBasePawn():ACharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseCollision= Cast<UCapsuleComponent>(RootComponent);
	BaseCollision->SetCollisionProfileName(CollisionProfiles::PawnBlockAllButSelf);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
}

void AKhaosBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	Subsystem->ClearAllMappings();
 
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	SetupEnhancedPlayerInput(Subsystem, EnhancedInputComponent);
}

void AKhaosBasePawn::OnPlayerMovementAction(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AKhaosBasePawn::SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent)
{
	EnhancedInputSubsystem->AddMappingContext(DefaultInputConfig->DefaultMappingContext, 0);
 
	PlayerInputComponent->BindAction(DefaultInputConfig->DirectionalMovement.Get(), ETriggerEvent::Triggered, this, &AKhaosBasePawn::OnPlayerMovementAction);
}
