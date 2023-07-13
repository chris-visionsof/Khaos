// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPersonPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "FirstPersonInputDataConfig.h"
#include "Khaos/Public/KhaosDefinitions.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

AFirstPersonPlayerPawn::AFirstPersonPlayerPawn() : AKhaosBasePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	PhysicsHandleGrabReference = CreateDefaultSubobject<USceneComponent>("PhysicsHandleGrabReference");
	PhysicsHandleGrabReference->SetupAttachment(FirstPersonCamera);
	PhysicsHandleGrabReference->SetRelativeLocation({ActionableItemTraceLength, 0.0, 0.0});

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandle");
}

void AFirstPersonPlayerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocationAndRotation(PhysicsHandleGrabReference->GetComponentLocation(), PhysicsHandleGrabReference->GetComponentRotation());
	}
}

void AFirstPersonPlayerPawn::OnPlayerActionedActor_Implementation(AKhaosActionableActor* ActionedActor)
{
	ActionedActor->PerformAction();
}

void AFirstPersonPlayerPawn::OnPlayerLookAndMotionInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFirstPersonPlayerPawn::OnPlayerMovementInput(bool bReverse, UE::Math::TVector<double>(AActor::*VectorFunc)() const)
{
	if (Controller != nullptr)
	{
		AddMovementInput((this->*VectorFunc)(), (bReverse ? KeyboardInputMovementValue : (KeyboardInputMovementValue * -1)));
	}
}

void AFirstPersonPlayerPawn::OnPlayerPerformActionInput()
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

		if(!IsValid(Hit.GetActor())) { return; }

		AActor* HitActor = Hit.GetActor();
		TArray<FName>& HitActorTags = HitActor->Tags;
		if(HitActorTags.Contains(ActorTags::Grabbable))
		{
			FRotator NormalRot = FRotationMatrix::MakeFromX(Hit.Normal).Rotator();
			PhysicsHandle->GrabComponentAtLocationWithRotation(Hit.GetComponent(), TEXT("None"), Hit.Location, NormalRot);
			PhysicsHandleGrabReference->SetWorldTransform(FTransform(NormalRot, Hit.Location));
		}

		if(HitActorTags.Contains(ActorTags::Actionable)
			&& HitActor->IsA(AKhaosActionableActor::StaticClass()))
		{
			OnPlayerActionedActor(Cast<AKhaosActionableActor>(HitActor));
		}
	}
}

void AFirstPersonPlayerPawn::OnPlayerPerformActionInputRelease()
{
	if(PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->ReleaseComponent();
	}
}

void AFirstPersonPlayerPawn::SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent)
{
	Super::SetupEnhancedPlayerInput(EnhancedInputSubsystem, PlayerInputComponent);

	EnhancedInputSubsystem->AddMappingContext(FirstPersonInputConfig->MappingContext, 1);

	PlayerInputComponent->BindAction(FirstPersonInputConfig->LookAndMotion.Get(), ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerLookAndMotionInput);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->MoveForward, ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerMovementInput, true, &AActor::GetActorForwardVector);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->MoveBackward, ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerMovementInput, false, &AActor::GetActorForwardVector);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->StrifeRight, ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerMovementInput, true, &AActor::GetActorRightVector);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->StrifeLeft, ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerMovementInput, false, &AActor::GetActorRightVector);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->PerformAction, ETriggerEvent::Triggered, this, &AFirstPersonPlayerPawn::OnPlayerPerformActionInput);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->PerformAction, ETriggerEvent::Completed, this, &AFirstPersonPlayerPawn::OnPlayerPerformActionInputRelease);
	PlayerInputComponent->BindAction(FirstPersonInputConfig->PerformAction, ETriggerEvent::Canceled, this, &AFirstPersonPlayerPawn::OnPlayerPerformActionInputRelease);
}



