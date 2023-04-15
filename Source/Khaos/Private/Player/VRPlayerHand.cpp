#include "VRPlayerHand.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Interactivity/GrabbableComponent.h"

AVRPlayerHand::AVRPlayerHand()
{
	MotionControllerComponent = CreateDefaultSubobject<UOpenXRHandMotionController>("MotionController");
	MotionControllerComponent->OnMotionControllerGraspedUpdated.AddDynamic(this, &AVRPlayerHand::OnMotionControllerGraspedUpdated);
	SetRootComponent(MotionControllerComponent);
	AddInstanceComponent(MotionControllerComponent);

	HandRoot = CreateDefaultSubobject<USphereComponent>("HandRoot");
	HandRoot->SetupAttachment(MotionControllerComponent);
	AddInstanceComponent(HandRoot);

	RootConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>("RootConstraint");
	RootConstraint->SetupAttachment(HandRoot);
	RootConstraint->ComponentName1 = FConstrainComponentPropName { "RootBoxCollision" };
	RootConstraint->ComponentName2 = FConstrainComponentPropName { "HandRoot" };

	RootBoxCollision = CreateDefaultSubobject<UBoxComponent>("RootBoxCollision");
	RootBoxCollision->SetupAttachment(HandRoot);
	AddInstanceComponent(RootBoxCollision);

	CoreHandCollision = CreateDefaultSubobject<UStaticMeshComponent>("CoreHandCollision");
	CoreHandCollision->SetupAttachment(RootBoxCollision);
	AddInstanceComponent(CoreHandCollision);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("HandSkeletalMesh");
	SkeletalMeshComponent->SetupAttachment(CoreHandCollision);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	SkeletalMeshComponent->SetGenerateOverlapEvents(true);
	SkeletalMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AVRPlayerHand::OnHandBeginOverlap);
	SkeletalMeshComponent->OnComponentEndOverlap.AddDynamic(this, &AVRPlayerHand::OnHandEndOverlap);
	AddInstanceComponent(SkeletalMeshComponent);
}

void AVRPlayerHand::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MotionControllerComponent->bRight = bRightHand;
}

void AVRPlayerHand::OnMotionControllerGraspedUpdated(UOpenXRHandMotionController* GraspedController, bool bGrasped)
{
	if(bGrasped == false) { return; }

	if(!OverlappingActor.IsSet()) { return; }

	OnPlayerGraspedActor(this, OverlappingActor.GetValue().Key, OverlappingActor.GetValue().Value);
}

void AVRPlayerHand::OnHandBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if(!IsValid(OtherActor)) { return; }

	if(!OverlappedComponent->IsA(UGrabbableComponent::StaticClass()))
	{
		UGrabbableComponent* GrabbableComp = Cast<UGrabbableComponent>(OtherComp);
		OverlappingActor = TTuple<TObjectPtr<UGrabbableComponent>, TObjectPtr<AActor>>(GrabbableComp, OtherActor);
	}
}

void AVRPlayerHand::OnHandEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32)
{
	if(!IsValid(OtherActor) || !OverlappingActor.IsSet()) { return; }

	if(!OverlappedComponent->IsA(UGrabbableComponent::StaticClass())
		&& OtherActor == OverlappingActor.GetValue().Value.Get())
	{
		OverlappingActor.Reset();
	}
}

