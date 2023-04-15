// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.generated.h"

UENUM()
enum class EGrabbableBounds
{
	NotSet,
	Sphere,
	Box,
	Capsule 
};

UENUM()
enum class EGrabState
{
	Idle,
	Grabbable,
	Grabbed
};

UCLASS(Abstract, ShowCategories= (Collision, Rendering))
class KHAOS_API UGrabbableComponent : public UPrimitiveComponent
{
public:
	UGrabbableComponent();

private:
	GENERATED_BODY()

public:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override PURE_VIRTUAL(UGrabbableComponent::CalcBounds, return Super::CalcBounds(LocalToWorld););
	
	// VISIONS MOD
	// Overriden to allow for the interactivity mode to override visibility  
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	// VISIONS MOD END

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual UBodySetup* GetBodySetup() override;

	virtual FCollisionShape GetCollisionShape(float Inflation) const override PURE_VIRTUAL(UGrabbableComponent::GetCollisionShape, return Super::GetCollisionShape(Inflation););

	EGrabbableBounds GetGrabbableBounds() const { return GrabbableBounds; }

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	
protected:
	virtual void CompleteBodySetup(UBodySetup* const InBodySetup) const PURE_VIRTUAL(UGrabbableComponent::CompleteBodySetup, );


	virtual void OnRegister() override;

	virtual void SetGrabState(EGrabState NewGrabState);
	
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnUnregister() override;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	virtual void RenderCheck();
#endif
	
	bool IsInteractivityEnabled() const;

	virtual void UpdateBodySetup() PURE_VIRTUAL(UGrabbableComponent::UpdateBodySetup, );

	virtual void UpdateCollisionProfile() override;

	bool bInteractivityPreviewCache;

	UPROPERTY()
	EGrabbableBounds GrabbableBounds;

	UPROPERTY()
	EGrabState GrabState;

	FTimerHandle InteractivityTimerHandle;
	
	/** Description of collision */
	UPROPERTY(transient, duplicatetransient)
	TObjectPtr<UBodySetup> BodySetup;

private:

	class FGrabbableSceneProxy final : public FPrimitiveSceneProxy 
	{
	public:
		FGrabbableSceneProxy(UGrabbableComponent* InComponent);

		virtual SIZE_T GetTypeHash() const override;

		virtual uint32 GetMemoryFootprint() const override;

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	protected:

		FORCEINLINE static bool InView(const FVector& Location, const FVector& Scale, const FSceneView* View)
		{
			return View ? View->ViewFrustum.IntersectBox(Location, (Scale / 2)) : false;
		}
		
		TObjectPtr<UGrabbableComponent> RepresentativeComponent;
		bool bInteractivityPreview;
	};
	
	friend FGrabbableSceneProxy;
};


