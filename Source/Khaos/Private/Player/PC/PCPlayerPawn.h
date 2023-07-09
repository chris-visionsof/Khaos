// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/KhaosBasePawn.h"
#include "PCPlayerPawn.generated.h"

UCLASS()
class KHAOS_API APCPlayerPawn : public AKhaosBasePawn
{
	GENERATED_BODY()

public:
	APCPlayerPawn();

	virtual void Tick(float DeltaSeconds) override;

protected:
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
	TObjectPtr<class UPCInputDataConfig> PCInputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interactivity")
	double ActionableItemTraceLength = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	float KeyboardInputMovementValue = 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool DebugTrace = false;
#endif
};
