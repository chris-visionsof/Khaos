// Fill out your copyright notice in the Description page of Project Settings.

#include "VRPlayerPawn.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Utils/Debugging/VRDebugTextCollectionActor.h"

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

	auto HandFunc = [this](TObjectPtr<UChildActorComponent>& HandToSetup, bool bIsRight = true)
	{
		HandToSetup = NewObject<UChildActorComponent>(this, (bIsRight ? "RightHand": "LeftHand"));
		HandToSetup->SetChildActorClass((bIsRight ? RightHandComponentType : LeftHandComponentType));
		HandToSetup->RegisterComponent();
		HandToSetup->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	};

	HandFunc(RightHandComponent);
	HandFunc(LeftHandComponent, false);
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


