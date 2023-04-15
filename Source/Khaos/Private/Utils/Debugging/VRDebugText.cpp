// Fill out your copyright notice in the Description page of Project Settings.


#include "VRDebugText.h"

#include "HeadMountedDisplayTypes.h"
#include "IXRTrackingSystem.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AVRDebugText::AVRDebugText()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	DebugText = CreateDefaultSubobject<UTextRenderComponent>("DebugText");
	DebugText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DebugText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
   	DebugText->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	DebugText->TextMaterial = ConstructorHelpers::FObjectFinderOptional<UMaterial>(TEXT("/Game/_Shared/Debug/DebugTextMaterial")).Get();
	DebugText->TextRenderColor = FColor::Red;
	DebugText->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WorldSizeCache = DebugText->WorldSize;
}

void AVRDebugText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(IsActorBeingDestroyed()) { return; }
	
	UWorld* World = GetWorld();
	IXRTrackingSystem* TrackingSystem = GEngine->XRSystem.Get();
	
	FXRHMDData HMDData;
	TrackingSystem->GetHMDData(World, HMDData);
	const FVector CurLoc = GetActorLocation();

	constexpr float ToleranceForChange = 0.1;
	if(!CachedHMDPos.Equals(HMDData.Position, ToleranceForChange)
		|| CachedHMDRot.Equals(HMDData.Rotation, ToleranceForChange)
		|| !CurLoc.Equals(CurLoc, ToleranceForChange)
		|| !CurLoc.Equals(UnobstructedPosition, ToleranceForChange))
	{
		FVector EyelineVector = (HMDData.Position - CurLoc);
		UnobstructedPosition = CurLoc;
		float Scale = 1.0f;
		
		FHitResult TraceHit;

		if(World->LineTraceSingleByChannel(TraceHit, HMDData.Position, CurLoc, ECollisionChannel::ECC_WorldStatic))
		{
			UnobstructedPosition = TraceHit.Location;
			// Quick Forced Perspective
			Scale = (HMDData.Position - UnobstructedPosition).Length() / EyelineVector.Length();
		}

		EyelineVector.Normalize();
		UnobstructedPosition += (EyelineVector * PositionPadding);
		
		DebugText->SetWorldLocation(UnobstructedPosition);
		DebugText->SetWorldSize(Scale * WorldSizeCache);

		if(bRotate)
		{
			DebugText->SetWorldRotation(EyelineVector.Rotation());
		}
	}

	CachedHMDPos = HMDData.Position;
	CachedHMDRot = HMDData.Rotation;
}

float AVRDebugText::GetWorldSize()
{
	return DebugText->WorldSize;
}

void AVRDebugText::SetHorizontalAlignment(const EHorizTextAligment& Alignment)
{
	DebugText->HorizontalAlignment = Alignment;
}

void AVRDebugText::SetText(const FText& Text)
{
	DebugText->SetText(Text);
}

void AVRDebugText::SetWorldSize(const float& Size)
{
	DebugText->SetWorldSize(Size);
	WorldSizeCache = Size;
}

void AVRDebugText::SetShouldRotate(const bool& bInRotate)
{
	bRotate = bInRotate;
}

void AVRDebugText::SetTextRenderColor(const FColor& Color)
{
	DebugText->SetTextRenderColor(Color);
}
