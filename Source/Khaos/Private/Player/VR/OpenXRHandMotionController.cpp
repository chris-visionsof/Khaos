// Fill out your copyright notice in the Description page of Project Settings.

#include "OpenXRHandMotionController.h"

#include "IHeadMountedDisplay.h"
#include "Utils/Debugging/DebugTextChildComponent.h"

namespace 
{
#define FINGER_PAIR(Finger) \
	TTuple<int32, int32>(static_cast<int32>(EHandKeypoint::##Finger##Metacarpal), static_cast<int32>(EHandKeypoint::##Finger##Tip)) \

	TArray FINGER_INDEXES = {
		FINGER_PAIR(Thumb),
		FINGER_PAIR(Index),
		FINGER_PAIR(Middle),
		FINGER_PAIR(Ring),
		FINGER_PAIR(Little)
	};
#undef FINGER_PAIR

	constexpr bool bPERSISTENT_LINES = false;
	constexpr float DEBUG_LIFE_TIME = -1.0f;
	constexpr uint8 DEPTH_PRIORITY = ESceneDepthPriorityGroup::SDPG_MAX;
	constexpr float THICKNESS = 0.05f;
}

UOpenXRHandMotionController::UOpenXRHandMotionController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FTransform UOpenXRHandMotionController::GetIndexFingerTipPosition()
{
	const TTuple<int32, int32> CurFingerIndex = FINGER_INDEXES[EFingers::Index];

	if(CurrentHandControllerDataCache.HandKeyRotations.IsEmpty())
	{
		return FTransform();
	}

	const UE::Math::TQuat<double> Rotation = CurrentHandControllerDataCache.HandKeyRotations[CurFingerIndex.Value];
	const FVector Position = CurrentHandControllerDataCache.HandKeyPositions[CurFingerIndex.Value] + Rotation.RotateVector(IndexPositionOffset);

	return FTransform(Rotation,Position);
}

void UOpenXRHandMotionController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	checkf(TrackingSystem, TEXT("Tracking sysstem should not be null for a VR game"));

	TrackingSystem->GetMotionControllerData(GetWorld(), bRight ? EControllerHand::Right : EControllerHand::Left, CurrentHandControllerDataCache);
	
	if (!CurrentHandControllerDataCache.bValid) { return; }
	
	SetWorldTransform(FTransform(
		CurrentHandControllerDataCache.GripRotation.GetNormalized(),
		CurrentHandControllerDataCache.GripPosition,
		FVector(1.0f, 1.0f, 1.0f)));

	if(OnMotionControllerUpdate.IsBound())
	{
		OnMotionControllerUpdate.Broadcast(this, CurrentHandControllerDataCache);
	}

	CalculateFingers();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if(bRenderDebugAxes)
	{
		RenderDebugAxes();
	}

	if(bRenderDebugHand)
	{
		RenderDebugHand();
	}
#endif
}



void UOpenXRHandMotionController::BeginPlay()
{
	Super::BeginPlay();

	TrackingSystem = GEngine->XRSystem.Get();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if(bRenderDebugGrasp)
	{
		for (int i = 0; i < 5; ++i)
		{
			TObjectPtr<UDebugTextChildComponent> FingerDistTextRender = NewObject<UDebugTextChildComponent>(
				GetOwner(), FName("FingerDistance", i));
			FingerDistTextRender->RegisterComponent();
			FingerDistTextRender->SetWorldSize(0.8f);
			FingerDistTextRender->SetPositionPadding(5.0f);
			FingerClosedDistanceRenderTexts.Add(FingerDistTextRender);
		}
	}
#endif
}

bool UOpenXRHandMotionController::CalculateFingers()
{
	for (int i = 0; i < FINGER_INDEXES.Num(); ++i)
	{
		const EFingers CurFinger = static_cast<EFingers>(i);
		const TTuple<int32, int32> CurFingerIndex = FINGER_INDEXES[i];
		FFingerRange CurFingerRange = FingerRanges[CurFinger];
		
		const FVector StartPos = CurrentHandControllerDataCache.HandKeyPositions[CurFingerIndex.Key];
		const FVector EndPos = CurrentHandControllerDataCache.HandKeyPositions[CurFingerIndex.Value];
		CurFingerRange.CurrentDistance = FVector::Distance(StartPos, EndPos);
		
		FingerRangePercentages[i] = CurFingerRange.PercentageOfClosedRange();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if(bRenderDebugGrasp)
		{
			const FColor RenderColor = FColor::Green;
			FingerClosedDistanceRenderTexts[i]->SetWorldLocation(EndPos + CurrentHandControllerDataCache.GripRotation.GetNormalized().RotateVector(FVector(1, 3, -1)));
			FingerClosedDistanceRenderTexts[i]->SetText(FText::FormatOrdered(FText::FromString("{0}"), CurFingerRange.CurrentDistance));
			FingerClosedDistanceRenderTexts[i]->SetTextRenderColor(RenderColor);
			
			DrawDebugLine(GetWorld(), StartPos, EndPos, RenderColor, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, (THICKNESS * 2.5));
		}
#endif
	}

	return false;
}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
void UOpenXRHandMotionController::RenderDebugAxes()
{
	constexpr double DirectionalVectorLength = 3.0;
	const FVector LocalOrigin = GetComponentLocation();
	
	FVector LocalZVector(0, 0, DirectionalVectorLength);
	FVector LocalYVector(0, DirectionalVectorLength, 0);
	FVector LocalXVector(DirectionalVectorLength,0, 0);
	
	// Draw Local Origin 
	DrawDebugPoint(GetWorld(), LocalOrigin, 20, FColor::Black, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY);

	// Draw Axes 
#define DRAW_AXIS(Color, Vector) \
	Vector = CurrentHandControllerDataCache.GripRotation.GetNormalized().RotateVector(Vector); \
	DrawDebugLine(GetWorld(), LocalOrigin, LocalOrigin + Vector, FColor::Color, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS); \
	DrawDebugCone(GetWorld(), LocalOrigin + Vector, (Vector * -1), 1.0f, 0.78f, 0.78f, 6, FColor::Color, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS); \

	DRAW_AXIS(Red, LocalXVector)
	DRAW_AXIS(Green, LocalYVector)
	DRAW_AXIS(Blue, LocalZVector)
#undef DRAW_AXIS
}

void UOpenXRHandMotionController::RenderDebugHand()
{
	const FColor HandColor = FColor::Magenta;

	// Draw 0 - 1 (palm to wrist)
	DrawDebugLine(GetWorld(), CurrentHandControllerDataCache.HandKeyPositions[0], CurrentHandControllerDataCache.HandKeyPositions[1], HandColor, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS);

	for (const TTuple<int32, int32> CurFingerIndex : FINGER_INDEXES)
	{
		if ((EHandKeypointCount == CurrentHandControllerDataCache.HandKeyPositions.Num()) && (EHandKeypointCount == CurrentHandControllerDataCache.HandKeyRadii.Num()))
		{
			// Draw 1 - LineStart (wrist to first part of finger)
			DrawDebugLine(GetWorld(), CurrentHandControllerDataCache.HandKeyPositions[1], CurrentHandControllerDataCache.HandKeyPositions[CurFingerIndex.Key], HandColor, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS);
			
			// Iterate from FingerStart to Finger End
			for (int DigitIndex = CurFingerIndex.Key; DigitIndex < CurFingerIndex.Value; ++DigitIndex)
			{
				DrawDebugLine(GetWorld(), CurrentHandControllerDataCache.HandKeyPositions[DigitIndex], CurrentHandControllerDataCache.HandKeyPositions[DigitIndex + 1], HandColor, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS);
				DrawDebugSphere(GetWorld(), CurrentHandControllerDataCache.HandKeyPositions[DigitIndex+1], CurrentHandControllerDataCache.HandKeyRadii[DigitIndex+1], 4, HandColor, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS);
			}
		}
	}
}
#endif

