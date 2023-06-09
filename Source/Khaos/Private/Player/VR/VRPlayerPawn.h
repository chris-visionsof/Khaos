// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OpenXRHandMotionController.h"
#include "VRPlayerHand.h"
#include "Player/KhaosBasePawn.h"

#include "VRPlayerPawn.generated.h"

UCLASS(ClassGroup=VR)
class KHAOS_API AVRPlayerPawn : public AKhaosBasePawn 
{
	GENERATED_BODY()

public:
	AVRPlayerPawn();

	virtual void DisplayDebugText(const FText& DebugMessage, const float& TimeToDisplay=5.0f, const FColor& DisplayColor=FColor::Red);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="VR|Debug", DisplayName="Display Debug Text", meta=(AdvancedDisplay=1))
	virtual void DisplayDebugText_BP(FText DebugMessage, float TimeToDisplay=5.0f, FColor DisplayColor=FColor::Red);

	void OnPlayerFingerTouchInput(const FInputActionValue& Value, bool bRight, EFingers FingerTouched);

	virtual void SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR", DisplayName="Camera")
	TEnumAsByte<EHMDTrackingOrigin::Type> VRTrackingOrigin = EHMDTrackingOrigin::Floor;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TObjectPtr<UChildActorComponent> DebugVRHudComp;
	TObjectPtr<class AVRDebugTextCollectionActor> DebugVRHud;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="VR Root Position")
	TObjectPtr<USphereComponent> VRRootPosition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="Left Hand Component Type", Category="VR|Hands")
	TSubclassOf<AVRPlayerHand> LeftHandComponentType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Left Hand", Category="VR|Hands")
	TObjectPtr<UChildActorComponent> LeftHandComponent;

	UPROPERTY(BlueprintReadOnly, DisplayName="Left Hand Actor", Category="VR|Hands")
	TObjectPtr<AVRPlayerHand> LeftHandActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, DisplayName="Right Hand Component Type", Category="VR|Hands")
	TSubclassOf<AVRPlayerHand> RightHandComponentType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Right Hand", Category="VR|Hands")
	TObjectPtr<UChildActorComponent> RightHandComponent;

	UPROPERTY(BlueprintReadOnly, DisplayName="Right Hand Actor", Category="VR|Hands")
	TObjectPtr<AVRPlayerHand> RightHandActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UVRInputConfigData> VRInputConfig;

	UPROPERTY(BlueprintReadOnly, meta=(Bitmask, BitmaskEnum = EFingers))
	int32 RightFingersTouched;

	UPROPERTY(BlueprintReadOnly, meta=(Bitmask, BitmaskEnum = EFingers))
	int32 LeftFingersTouched;
};
