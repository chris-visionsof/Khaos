// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabbableSphereComponent.h"

UGrabbableSphereComponent::UGrabbableSphereComponent():
	UGrabbableComponent()
{
	GrabbableBounds = EGrabbableBounds::Sphere;
}

FCollisionShape UGrabbableSphereComponent::GetCollisionShape(float Inflation) const
{
	const float CollisionRad = FMath::Max(GetRelativeScale3D().X + Inflation, 0.0f);

	return FCollisionShape::MakeSphere(CollisionRad);
}

void UGrabbableSphereComponent::UpdateBodySetup()
{
	FKSphereElem* SphereElem = BodySetup->AggGeom.SphereElems.GetData();

	double XScale = GetRelativeScale3D().X;
	if (XScale < KINDA_SMALL_NUMBER)
	{
		XScale = 1.0f;
	}

	SphereElem->SetTransform(FTransform::Identity);
	SphereElem->Radius = XScale;
}

void UGrabbableSphereComponent::CompleteBodySetup(UBodySetup* const InBodySetup) const
{
	InBodySetup->AggGeom.SphereElems.Add(FKSphereElem());
}

FBoxSphereBounds UGrabbableSphereComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const float Radius = FMath::Max(GetRelativeScale3D().X, 0.0f);
	
	return FBoxSphereBounds( FVector::ZeroVector, FVector(Radius), Radius).TransformBy(LocalToWorld);
}
