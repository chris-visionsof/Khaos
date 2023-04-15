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
	void OnPlayerGraspedActor(AVRPlayerHand* GraspingHand, UGrabbableComponent* GraspedComp, AActor* GraspedActor);

	UPROPERTY(EditAnywhere, DisplayName="Is Right Hand", Category="VR|Hands")
	bool bRightHand = false;

protected:
	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual void OnMotionControllerGraspedUpdated(UOpenXRHandMotionController* GraspedController, bool bGrasped);

	UFUNCTION()
	virtual void OnHandBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnHandEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	OptionalGraspedPair OverlappingActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Motion Controller", Category="Hands")
	TObjectPtr<UOpenXRHandMotionController> MotionControllerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Hand Root", Category="VR|Hands")
	TObjectPtr<class USphereComponent> HandRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Motion Controller", Category="VR|Hands")
	TObjectPtr<UPhysicsConstraintComponent> RootConstraint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Root Box Collision", Category="VR|Hands")
	TObjectPtr<class UBoxComponent> RootBoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Core Hand Collision", Category="VR|Hands")
	TObjectPtr<UStaticMeshComponent> CoreHandCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Hand SkeletalMesh", Category="VR|Hands")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
};
