// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HandDefininitions.h"
#include "IXRTrackingSystem.h"

#include "OpenXRHandMotionController.generated.h"


USTRUCT(BlueprintType)
struct FFingerRange
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MaxDistance = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float MinDistance = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentDistance = 0.0f;

	float PercentageOfClosedRange() const
	{
		return 1 - (CurrentDistance - MinDistance) / (MaxDistance - MinDistance);
	}
};

UCLASS(ClassGroup=(VR), meta=(BlueprintSpawnableComponent, DisplayName="OpenXR Hand Motion Controller"))
class KHAOS_API UOpenXRHandMotionController : public USceneComponent 
{
	friend class AVRPlayerHand;

	GENERATED_BODY()

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMotionControllerUpdated, UOpenXRHandMotionController*, GraspedController, FXRMotionControllerData, MotionControllerData);
	
public:
	UOpenXRHandMotionController();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called when the motion controller data has updated */
	UPROPERTY(BlueprintAssignable, Category = "VR")
	FOnMotionControllerUpdated OnMotionControllerUpdate;

	/** Is the controller tracked on the right side? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR")
	bool bRight;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual bool CalculateGrasped();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	virtual void RenderDebugAxes();

	virtual void RenderDebugHand();
#endif

	class IXRTrackingSystem* TrackingSystem;

	FXRMotionControllerData CurrentHandControllerDataCache;

	/** The configuration of the finger ranges for use in to determine if the hand is grasped and animation blending **/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="VR")
	TMap<TEnumAsByte<EFingers>, FFingerRange> FingerRanges = {
		{ EFingers::Thumb, FFingerRange() },
		{ EFingers::Index,  FFingerRange() },
		{ EFingers::Middle, FFingerRange() },
		{ EFingers::Ring, FFingerRange() },
		{ EFingers::Little, FFingerRange() },
	};

	/** The percentage that each finger is within of it's flexed range  **/
	UPROPERTY(BlueprintReadOnly, Category="VR")
	TArray<float> FingerRangePercentages = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Debug")
	bool bRenderDebugAxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Debug")
	bool bRenderDebugHand;

	bool bRenderDebugGrasp;
#endif

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TArray<TObjectPtr<class UDebugTextChildComponent>> FingerClosedDistanceRenderTexts;
#endif
};
