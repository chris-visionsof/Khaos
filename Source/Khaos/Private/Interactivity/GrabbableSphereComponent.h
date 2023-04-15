// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.h"
#include "GrabbableSphereComponent.generated.h"

UCLASS(ClassGroup=(Interactivity), meta=(BlueprintSpawnableComponent))
class KHAOS_API UGrabbableSphereComponent : public UGrabbableComponent
{
	GENERATED_BODY()

	UGrabbableSphereComponent();

public:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual FCollisionShape GetCollisionShape(float Inflation) const override;

protected:
	virtual void UpdateBodySetup() override;

	virtual void CompleteBodySetup(UBodySetup* const BodySetup) const override;
};
