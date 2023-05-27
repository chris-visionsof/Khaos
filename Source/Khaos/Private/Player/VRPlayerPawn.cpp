// Fill out your copyright notice in the Description page of Project Settings.

#include "VRPlayerPawn.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputConfigData.h"
#include "KhaosDefinitions.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Utils/Debugging/VRDebugTextCollectionActor.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"

// Sets default values
AVRPlayerPawn::AVRPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootMovementCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootMovementCollision"));
	RootMovementCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RootMovementCollision->SetCollisionProfileName(CollisionProfiles::PawnBlockAllButSelf);
	RootMovementCollision->InitCapsuleSize(35.0f, 90.0f);
	AddInstanceComponent(RootMovementCollision);
	SetRootComponent(RootMovementCollision);

	VRRootPosition = CreateDefaultSubobject<USphereComponent>(TEXT("VRRootPosition"));
	VRRootPosition->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	VRRootPosition->SetSphereRadius(16.0f, false);
	VRRootPosition->SetupAttachment(RootMovementCollision);
	VRRootPosition->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(VRRootPosition);
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
	DebugVRHud->SetOwner(this);
	DebugVRHud->SetGeneralWorldSize(2.0f);
#endif

	auto HandSetupFunc = [this](TObjectPtr<UChildActorComponent>& HandToSetup, TObjectPtr<AVRPlayerHand>& ActorRef, bool bIsRight = true)
	{
		HandToSetup = NewObject<UChildActorComponent>(this, (bIsRight ? "RightHand": "LeftHand"));
		HandToSetup->SetChildActorClass((bIsRight ? RightHandComponentType : LeftHandComponentType));
		HandToSetup->RegisterComponent();
		HandToSetup->AttachToComponent(VRRootPosition, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ActorRef = Cast<AVRPlayerHand>(HandToSetup->GetChildActor());
		ActorRef->SetOwner(this);
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

void AVRPlayerPawn::OnPlayerFingerTouchAction(const FInputActionValue& ActionValue, bool bRight, EFingers FingerTouched) 
{
	constexpr static int32 Mask = (EFingers::Index | EFingers::Thumb);
	int32& Fingers = bRight ? RightFingersTouched : LeftFingersTouched;
	const TObjectPtr<AVRPlayerHand> HandActor = bRight ? RightHandActor : LeftHandActor;

	if(ActionValue.Get<bool>())
	{
		Fingers |= FingerTouched;
	}
	else
	{
		Fingers &= ~FingerTouched;
	}

	if(HandActor->IsGrasped() && ((Fingers & Mask) != Mask))
	{
 		HandActor->OnPlayerReleaseAction();
	}
	else if(!HandActor->IsGrasped() && ((Fingers & Mask) == Mask))
	{
		HandActor->OnPlayerGrabAction();
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

	PEI->BindAction(InputActions->ThumbTouch_R.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, true, EFingers::Thumb);
	PEI->BindAction(InputActions->ThumbTouch_L.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, false, EFingers::Thumb);
	PEI->BindAction(InputActions->ThumbTouch_R.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, true, EFingers::Thumb);
	PEI->BindAction(InputActions->ThumbTouch_L.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, false, EFingers::Thumb);

	PEI->BindAction(InputActions->IndexTouch_R.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, true, EFingers::Index);
	PEI->BindAction(InputActions->IndexTouch_L.Get(), ETriggerEvent::Triggered, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, false, EFingers::Index);
	PEI->BindAction(InputActions->IndexTouch_R.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, true, EFingers::Index);
	PEI->BindAction(InputActions->IndexTouch_L.Get(), ETriggerEvent::Completed, this, &AVRPlayerPawn::OnPlayerFingerTouchAction, false, EFingers::Index);
}


