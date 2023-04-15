// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "OpenXRHandMotionController.h"
#include "VRPlayerHand.h"

#include "VRPlayerPawn.generated.h"

typedef TOptional<TTuple<TObjectPtr<class UGrabbableComponent>, TObjectPtr<AActor>>> OptionalGraspedPair;

UCLASS(ClassGroup=VR)
class KHAOS_API AVRPlayerPawn : public APawn 
{
	GENERATED_BODY()

public:
	AVRPlayerPawn();

	virtual void DisplayDebugText(const FText& DebugMessage, const float& TimeToDisplay=5.0f, const FColor& DisplayColor=FColor::Red);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="VR|Debug", DisplayName="Display Debug Text", meta=(AdvancedDisplay=1))
	virtual void DisplayDebugText_BP(FText DebugMessage, float TimeToDisplay=5.0f, FColor DisplayColor=FColor::Red);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Camera")
	TEnumAsByte<EHMDTrackingOrigin::Type> VRTrackingOrigin = EHMDTrackingOrigin::Floor;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TObjectPtr<UChildActorComponent> DebugVRHudComp;
	TObjectPtr<class AVRDebugTextCollectionActor> DebugVRHud;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="Left Hand Component Type", Category="VR|Hands")
	TSubclassOf<AVRPlayerHand> LeftHandComponentType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Left Hand", Category="VR|Hands")
	TObjectPtr<UChildActorComponent> LeftHandComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="Right Hand Component Type", Category="VR|Hands")
	TSubclassOf<AVRPlayerHand> RightHandComponentType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Right Hand", Category="VR|Hands")
	TObjectPtr<UChildActorComponent> RightHandComponent;
};
