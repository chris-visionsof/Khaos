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
	// Sets default values for this character's properties
	APCPlayerPawn();

protected:
	virtual void OnPlayerLookAndMotionAction(const struct FInputActionValue& Value);

	void virtual SetupEnhancedPlayerInput(UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem, UEnhancedInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UPCInputDataConfig> PCInputConfig;
};
