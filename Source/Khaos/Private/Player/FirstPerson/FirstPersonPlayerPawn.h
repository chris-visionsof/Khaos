// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Khaos/Public/Actors/KhaosActionableActor.h"
#include "Player/KhaosBasePawn.h"
#include "FirstPersonPlayerPawn.generated.h"

UCLASS()
class KHAOS_API AFirstPersonPlayerPawn : public AKhaosBasePawn
{
	GENERATED_BODY()

public:
	AFirstPersonPlayerPawn();

	virtual void Tick(float DeltaSeconds) override;

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerActionedActor(AKhaosActionableActor* ActionedActor);

	virtual void OnPlayerLookAndMotionInput(const struct FInputActionValue& Value);

	virtual void OnPlayerMovementInput(bool bReverse, UE::Math::TVector<double>(AActor::*VectorFunc)() const);

	virtual void OnPlayerPerformActionInput();

	virtual void OnPlayerPerformActionInputRelease();

	void virtual SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category="Interactivity")
	TObjectPtr<class USceneComponent> PhysicsHandleGrabReference;

	UPROPERTY(EditAnywhere, Category="Interactivity")
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UFirstPersonInputDataConfig> FirstPersonInputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactivity")
	double ActionableItemTraceLength = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	float KeyboardInputMovementValue = 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool DebugTrace = false;
#endif
};
