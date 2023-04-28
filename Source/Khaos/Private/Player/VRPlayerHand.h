// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OpenXRHandMotionController.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "VRPlayerHand.generated.h"

typedef TOptional<TTuple<TObjectPtr<class UGrabbableComponent>, TObjectPtr<AActor>>> OptionalGraspedPair;

UCLASS(ClassGroup=VR)
class KHAOS_API AVRPlayerHand : public AActor
{
	GENERATED_BODY()

public:
	AVRPlayerHand();

	UFUNCTION(BlueprintImplementableEvent, Category="VR")
	void OnPlayerGrabbedActorEvent(AVRPlayerHand* GraspingHand, UGrabbableComponent* GraspedComp, AActor* GraspedActor);

	UFUNCTION(BlueprintCallable)
	virtual void OnPlayerGrabAction();
	
	UFUNCTION(BlueprintCallable)
	virtual void OnPlayerReleaseAction();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="VR")
	TMap<TEnumAsByte<EFingers>, float> GetFingerRangePercentages();

	UPROPERTY(EditAnywhere, DisplayName="Is Right Hand", Category="VR|Hands")
	bool bRightHand = false;

	UPROPERTY(EditAnywhere, DisplayName="Finger Interperlation Speed", Category="VR|Hands")
	float FingerInterpSpeed = 10;

	/** Specifies how much further to extend the fingers collision position based on its normal **/
	UPROPERTY(EditAnywhere, DisplayName="Finger Collision Overshoot", Category="VR|Hands")
	float FingerCollisionOvershoot = .05;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnGrabBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnGrabBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void PostInitializeComponents() override;

	void PositionFingerCollisions() const;

	TArray<TArray<FVector, TInlineAllocator<5>>, TInlineAllocator<5>> FingerCollisionPositions;

	/** The percentage that each finger is within of it's flexed range that includes limiting based on collisions  **/
	UPROPERTY(BlueprintReadOnly, Category="VR")
	TMap<TEnumAsByte<EFingers>, float> FingerCollisionRangePercentages = {
		{ EFingers::Thumb, 0.0f },
		{ EFingers::Index, 0.0f },
		{ EFingers::Middle, 0.0f },
		{ EFingers::Ring, 0.0f },
		{ EFingers::Little, 0.0f },
	};

	TArray<float> FingerCollisionRangeTargetPercentages = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	TArray<float> FingerSplineTotalRanges = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	TArray<TObjectPtr<class USplineComponent>, TInlineAllocator<5>> SplineCollisionComponents;

	OptionalGraspedPair OverlappingActor;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Debug")
	bool bFingerCollisionDebugTracing;
#endif

	UPROPERTY(BlueprintReadOnly, DisplayName="Is Grasped", Category="VR|Hands")
	bool bIsGrasped = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Motion Controller", Category="Hands")
	TObjectPtr<UOpenXRHandMotionController> MotionControllerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Hand Root", Category="VR|Hands")
	TObjectPtr<class USphereComponent> HandRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Root Constraint", Category="VR|Hands")
	TObjectPtr<UPhysicsConstraintComponent> RootConstraint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Root Box Collision", Category="VR|Hands")
	TObjectPtr<class UBoxComponent> RootBoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Core Hand Collision", Category="VR|Hands")
	TObjectPtr<UStaticMeshComponent> CoreHandCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Hand Skeletal Mesh", Category="VR|Hands")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Grab Constraint", Category="VR|Hands")
	TObjectPtr<UPhysicsConstraintComponent> GrabConstraint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Grab Overlap Box", Category="VR|Hands")
	TObjectPtr<UBoxComponent> GrabOverlapBox;

	// Finger Collision
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Thumb Collision Postion", Category="VR|Hands")
	TObjectPtr<USceneComponent> ThumbCollisionPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Thumb Collision", Category="VR|Hands")
	TObjectPtr<class UCapsuleComponent> ThumbCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Index Collision Postion", Category="VR|Hands")
	TObjectPtr<USceneComponent> IndexCollisionPosition;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Index Collision", Category="VR|Hands")
	TObjectPtr<UCapsuleComponent> IndexCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Middle Collision Postion", Category="VR|Hands")
	TObjectPtr<USceneComponent> MiddleCollisionPosition;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Middle Collision", Category="VR|Hands")	
	TObjectPtr<UCapsuleComponent> MiddleCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Ring Collision Postion", Category="VR|Hands")
	TObjectPtr<USceneComponent> RingCollisionPosition;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Ring Collision", Category="VR|Hands")
	TObjectPtr<UCapsuleComponent> RingCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Little Collision Postion", Category="VR|Hands")
	TObjectPtr<USceneComponent> LittleCollisionPosition;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Little Collision", Category="VR|Hands")	
	TObjectPtr<UCapsuleComponent> LittleCollision;

	// Collision Spines

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Thumb Collision Spline", Category="VR|Hands")
	TObjectPtr<USplineComponent> ThumbCollisionSpline;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Index Collision Spline", Category="VR|Hands")
	TObjectPtr<USplineComponent> IndexCollisionSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Middle Collision Spline", Category="VR|Hands")
	TObjectPtr<USplineComponent> MiddleCollisionSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Ring Collision Spline", Category="VR|Hands")
	TObjectPtr<USplineComponent> RingCollisionSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Little Collision Spline", Category="VR|Hands")
	TObjectPtr<USplineComponent> LittleCollisionSpline;
};
