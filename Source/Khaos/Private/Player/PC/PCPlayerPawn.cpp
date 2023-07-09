// Fill out your copyright notice in the Description page of Project Settings.


#include "PCPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "PCInputDataConfig.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

APCPlayerPawn::APCPlayerPawn() : AKhaosBasePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	PhysicsHandleGrabReference = CreateDefaultSubobject<USceneComponent>("PhysicsHandleGrabReference");
	PhysicsHandleGrabReference->SetupAttachment(FirstPersonCamera);
	PhysicsHandleGrabReference->SetRelativeLocation({ActionableItemTraceLength, 0.0, 0.0});

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandle");
}

void APCPlayerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocationAndRotation(PhysicsHandleGrabReference->GetComponentLocation(), PhysicsHandleGrabReference->GetComponentRotation());
	}
}

void APCPlayerPawn::OnPlayerLookAndMotionInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APCPlayerPawn::OnPlayerMovementInput(bool bReverse, UE::Math::TVector<double>(AActor::*VectorFunc)() const)
{
	if (Controller != nullptr)
	{
		AddMovementInput((this->*VectorFunc)(), (bReverse ? KeyboardInputMovementValue : (KeyboardInputMovementValue * -1)));
	}
}

void APCPlayerPawn::OnPlayerPerformActionInput()
{
	if(PhysicsHandle->GrabbedComponent) { return; }

	const FTransform CameraTransform = FirstPersonCamera->GetComponentTransform();
	const FVector TraceEnd = CameraTransform.TransformPosition({ ActionableItemTraceLength, 0.0, 0.0});
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if(DebugTrace)
	{
		DrawDebugPoint(GetWorld(), TraceEnd, 10.0f, FColor::Red, false, 0.5f, ESceneDepthPriorityGroup::SDPG_MAX);
	}
#endif

	FHitResult Hit;
	FCollisionQueryParams TraceParams{};
	TraceParams.AddIgnoredActors(TArray<AActor*>{ this, GetOwner() });
	if(GetWorld()->LineTraceSingleByChannel(Hit, CameraTransform.GetLocation(), TraceEnd, ECC_WorldDynamic, TraceParams))
	{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if(DebugTrace)
		{
			DrawDebugPoint(GetWorld(), Hit.Location, 10.0f, FColor::Blue, false, 0.5f, ESceneDepthPriorityGroup::SDPG_MAX);
		}
#endif
		
		FRotator NormalRot = FRotationMatrix::MakeFromX(Hit.Normal).Rotator();
		PhysicsHandle->GrabComponentAtLocationWithRotation(Hit.GetComponent(), TEXT("None"), Hit.Location, NormalRot);
		PhysicsHandleGrabReference->SetWorldTransform(FTransform(NormalRot, Hit.Location));
	}
}

void APCPlayerPawn::OnPlayerPerformActionInputRelease()
{
	if(PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
	}
}

void APCPlayerPawn::SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent)
{
	Super::SetupEnhancedPlayerInput(EnhancedInputSubsystem, PlayerInputComponent);

	EnhancedInputSubsystem->AddMappingContext(PCInputConfig->PCMappingContext, 1);

	PlayerInputComponent->BindAction(PCInputConfig->LookAndMotion.Get(), ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerLookAndMotionInput);
	PlayerInputComponent->BindAction(PCInputConfig->MoveForward, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerMovementInput, true, &AActor::GetActorForwardVector);
	PlayerInputComponent->BindAction(PCInputConfig->MoveBackward, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerMovementInput, false, &AActor::GetActorForwardVector);
	PlayerInputComponent->BindAction(PCInputConfig->StrifeRight, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerMovementInput, true, &AActor::GetActorRightVector);
	PlayerInputComponent->BindAction(PCInputConfig->StrifeLeft, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerMovementInput, false, &AActor::GetActorRightVector);
	PlayerInputComponent->BindAction(PCInputConfig->PerformAction, ETriggerEvent::Triggered, this, &APCPlayerPawn::OnPlayerPerformActionInput);
	PlayerInputComponent->BindAction(PCInputConfig->PerformAction, ETriggerEvent::Completed, this, &APCPlayerPawn::OnPlayerPerformActionInputRelease);
	PlayerInputComponent->BindAction(PCInputConfig->PerformAction, ETriggerEvent::Canceled, this, &APCPlayerPawn::OnPlayerPerformActionInputRelease);
}



