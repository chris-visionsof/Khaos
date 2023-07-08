// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "KhaosBasePawn.generated.h"

UCLASS(Abstract)
class KHAOS_API AKhaosBasePawn : public ACharacter 
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AKhaosBasePawn();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void OnPlayerMovementAction(const struct FInputActionValue& Value);

	virtual void SetupEnhancedPlayerInput(class UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, class UEnhancedInputComponent* PlayerInputComponent);

	TObjectPtr<UCapsuleComponent> BaseCollision;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UInputConfigData> DefaultInputConfig;
};
