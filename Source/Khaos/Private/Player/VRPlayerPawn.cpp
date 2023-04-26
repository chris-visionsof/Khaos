// Fill out your copyright notice in the Description page of Project Settings.

#include "VRPlayerPawn.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputConfigData.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Utils/Debugging/VRDebugTextCollectionActor.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values
AVRPlayerPawn::AVRPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default Root
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	if(UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(VRTrackingOrigin);
		CameraComponent->bLockToHmd = true;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DebugVRHudComp = NewObject<UChildActorComponent>(this,"VRDebugHUD");
	DebugVRHudComp->SetChildActorClass(AVRDebugTextCollectionActor::StaticClass());
	DebugVRHudComp->RegisterComponent();
	DebugVRHudComp->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
	DebugVRHudComp->SetRelativeRotation(FRotator(0.0f, 180.0, 0.0f));
	DebugVRHudComp->SetRelativeLocation(FVector(40.0f, -15.0f, -15.0f));
	DebugVRHud = Cast<AVRDebugTextCollectionActor>(DebugVRHudComp->GetChildActor());
	DebugVRHud->SetGeneralWorldSize(2.0f);
#endif

	auto HandSetupFunc = [this](TObjectPtr<UChildActorComponent>& HandToSetup, TObjectPtr<AVRPlayerHand>& ActorRef, bool bIsRight = true)
	{
		HandToSetup = NewObject<UChildActorComponent>(this, (bIsRight ? "RightHand": "LeftHand"));
		HandToSetup->SetChildActorClass((bIsRight ? RightHandComponentType : LeftHandComponentType));
		HandToSetup->RegisterComponent();
		HandToSetup->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ActorRef = Cast<AVRPlayerHand>(HandToSetup->GetChildActor());
	};

	HandSetupFunc(RightHandComponent, RightHandActor);
	HandSetupFunc(LeftHandComponent, LeftHandActor, false);
}

void AVRPlayerPawn::DisplayDebugText(const FText& DebugMessage, const float& TimeToDisplay, const FColor& DisplayColor)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DebugVRHud->DisplayDebugText(DebugMessage, TimeToDisplay, DisplayColor);
#endif
}

void AVRPlayerPawn::DisplayDebugText_BP(FText DebugMessage, float TimeToDisplay, FColor DisplayColor)
{
	DisplayDebugText(DebugMessage, TimeToDisplay, DisplayColor);
}

void AVRPlayerPawn::OnPlayerGrabAction(const FInputActionValue&, bool bRight) 
{
	if(bRight)
	{
		RightHandActor->OnPlayerGrabAction();
	}
	else
	{
		LeftHandActor->OnPlayerGrabAction();
	}
}

void AVRPlayerPawn::OnPlayerReleaseAction(const FInputActionValue& Value, bool bRight)
{
	if(bRight)
	{
		RightHandActor->OnPlayerReleaseAction();
	}
	else
	{
		LeftHandActor->OnPlayerReleaseAction();
	}
}

void AVRPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
 
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputMappingContext, 0);
 
	// Get the EnhancedInputComponent
	UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	PEI->BindAction(InputActions->InputGrab_R.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerGrabAction, true);
	PEI->BindAction(InputActions->InputGrab_L.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerGrabAction, false);
	PEI->BindAction(InputActions->InputGrab_R.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerReleaseAction, true);
	PEI->BindAction(InputActions->InputGrab_L.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerReleaseAction, false);
}


