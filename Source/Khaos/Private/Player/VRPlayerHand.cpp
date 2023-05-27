#include "VRPlayerHand.h"

#include "KhaosDefinitions.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"

namespace 
{
	constexpr bool bPERSISTENT_LINES = false;
	constexpr float DEBUG_LIFE_TIME = -1.0f;
	constexpr uint8 DEPTH_PRIORITY = ESceneDepthPriorityGroup::SDPG_MAX;
	constexpr float THICKNESS = 0.05f;
}

AVRPlayerHand::AVRPlayerHand()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionControllerComponent = CreateDefaultSubobject<UOpenXRHandMotionController>("MotionController");
	SetRootComponent(MotionControllerComponent);

	HandRoot = CreateDefaultSubobject<USphereComponent>("HandRoot");
	HandRoot->SetupAttachment(MotionControllerComponent);

	RootBoxCollision = CreateDefaultSubobject<UBoxComponent>("RootBoxCollision");
	RootBoxCollision->SetCollisionProfileName(CollisionProfiles::PawnBlockAllButSelf);
	RootBoxCollision->SetSimulatePhysics(true);
	RootBoxCollision->SetupAttachment(HandRoot);

	RootConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>("RootConstraint");
	RootConstraint->SetupAttachment(HandRoot);
	RootConstraint->OverrideComponent1 = RootBoxCollision;
	RootConstraint->ComponentName1 = FConstrainComponentPropName { RootBoxCollision.GetFName() };
	RootConstraint->OverrideComponent2 = HandRoot;
	RootConstraint->ComponentName2 = FConstrainComponentPropName { HandRoot.GetFName() };
	
	CoreHandCollision = CreateDefaultSubobject<UStaticMeshComponent>("CoreHandCollision");
	CoreHandCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CoreHandCollision->SetCollisionProfileName(CollisionProfiles::PawnBlockAllButSelf);
	CoreHandCollision->SetupAttachment(RootBoxCollision);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("HandSkeletalMesh");
	SkeletalMeshComponent->SetupAttachment(CoreHandCollision);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GrabConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>("GrabConstraint");
	GrabConstraint->SetupAttachment(SkeletalMeshComponent);

	GrabOverlapBox = CreateDefaultSubobject<UBoxComponent>("GrabOverlapBox");
	GrabOverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GrabOverlapBox->SetCollisionProfileName(CollisionProfiles::InteractiveTrigger);
	GrabOverlapBox->SetupAttachment(SkeletalMeshComponent);

	auto CollisionSplineSetup = [this](TObjectPtr<USplineComponent>& SplineComponentToSetup, const FString& Name)
	{
		SplineComponentToSetup = CreateDefaultSubobject<USplineComponent>(FName(*FString::Printf(TEXT("%lsCollisionSpline"), *Name)));
		SplineComponentToSetup->SetupAttachment(SkeletalMeshComponent);
		SplineComponentToSetup->Duration = 1.0f;
		SplineCollisionComponents.Add(SplineComponentToSetup);
	};

	CollisionSplineSetup(ThumbCollisionSpline, "Thumb");
	CollisionSplineSetup(IndexCollisionSpline, "Index");
	CollisionSplineSetup(MiddleCollisionSpline, "Middle");
	CollisionSplineSetup(RingCollisionSpline, "Ring");
	CollisionSplineSetup(LittleCollisionSpline, "Little");

	auto FingerCollisionSetup = [this](TObjectPtr<USceneComponent>& PositionComp, TObjectPtr<UCapsuleComponent>& CollisionComp, const FString& Name)
	{
		PositionComp = CreateDefaultSubobject<USceneComponent>(FName(*FString::Printf(TEXT("%lsCollisionPosition"), *Name)));
		PositionComp->SetupAttachment(SkeletalMeshComponent);

		CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName(*FString::Printf(TEXT("%lsCollision"), *Name)));
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionComp->SetCollisionProfileName(CollisionProfiles::PawnBlockAllButSelf);
		CollisionComp->SetupAttachment(CoreHandCollision);
	};

	FingerCollisionSetup(ThumbCollisionPosition, ThumbCollision, "Thumb");
	FingerCollisionSetup(IndexCollisionPosition, IndexCollision, "Index");
	FingerCollisionSetup(MiddleCollisionPosition, MiddleCollision, "Middle");
	FingerCollisionSetup(RingCollisionPosition, RingCollision, "Ring");
	FingerCollisionSetup(LittleCollisionPosition, LittleCollision, "Little");

	PositionFingerCollisions();
}

bool AVRPlayerHand::GetHeldActor(FGrabbedActor& GrabbedActorOut)
{
	if(bIsGrasped)
	{
		GrabbedActorOut = HeldActor.GetValue();
		return true;
	}

	return false;
}

void AVRPlayerHand::OnPlayerGrabAction()
{
	if(bIsGrasped) { return; }

	TArray<UPrimitiveComponent*> OverlappingComps;
	GrabOverlapBox->GetOverlappingComponents(OverlappingComps);
	for (const UPrimitiveComponent* OverlappingComp : OverlappingComps)
	{
		// TODO: Probably shouldn't default to the root. Should probably take the grabbable component and go up the parent tree
		AActor* OwningActor = OverlappingComp->GetOwner();
		if(const auto RootPrimitive = Cast<UPrimitiveComponent>(OwningActor->GetRootComponent()))
		{
			GrabConstraint->OverrideComponent2 = SkeletalMeshComponent;
			GrabConstraint->ComponentName2 = FConstrainComponentPropName { SkeletalMeshComponent.GetFName() };
			GrabConstraint->OverrideComponent1 = RootPrimitive;
			GrabConstraint->ComponentName1 = FConstrainComponentPropName { RootPrimitive->GetFName() };
			GrabConstraint->InitComponentConstraint();
			bIsGrasped = true;

			auto GrabbedTrans = RootPrimitive->GetComponentTransform();
			GrabbedTrans.SetScale3D(FVector::One());

			HeldActor.Emplace(FGrabbedActor { OwningActor, RootPrimitive, GrabbedTrans.InverseTransformPosition(GrabConstraint->GetComponentLocation()) });

			break;
		}
	}
}

void AVRPlayerHand::OnPlayerReleaseAction()
{
	if(bIsGrasped)
	{
		GrabConstraint->BreakConstraint();
		HeldActor.Reset();
		bIsGrasped = false;
	}
}

void AVRPlayerHand::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FingerCollisionRangeTargetPercentages = MotionControllerComponent->FingerRangePercentages;

	for (int fingerIndex = 0; fingerIndex < 5; ++fingerIndex)
	{
		TArray<FVector, TInlineAllocator<5>> CurrentFingerCollisionPositions = FingerCollisionPositions[fingerIndex];
		float CurrentMotionControlFingerPercentage = MotionControllerComponent->FingerRangePercentages[fingerIndex];

		if(bFingerCollisionDebugTracing || CurrentMotionControlFingerPercentage > 0.05)
		{
			float FingerTraceLength = 0;

			for (int i = 0; i < (CurrentFingerCollisionPositions.Num() - 1); ++i)
			{
				FTransform SplineTransform = SplineCollisionComponents[fingerIndex]->GetComponentTransform();
				FVector	TraceStart = SplineTransform.TransformPosition(CurrentFingerCollisionPositions[i]),
						TraceEnd = SplineTransform.TransformPosition(CurrentFingerCollisionPositions[i+1]);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				if(bFingerCollisionDebugTracing)
				{
					DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Magenta, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY, THICKNESS);
				}
#endif

				FHitResult Hit;
				FCollisionQueryParams TraceParams{};
				TraceParams.AddIgnoredActors(TArray<AActor*>{ this, GetOwner() });
				if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParams))
				{
					FingerTraceLength += FVector::Distance(TraceStart, Hit.Location);

					float TraceFingerPercentage = FingerTraceLength / FingerSplineTotalRanges[fingerIndex];
					if(CurrentMotionControlFingerPercentage > TraceFingerPercentage)
					{
						FingerCollisionRangeTargetPercentages[fingerIndex] = TraceFingerPercentage + FingerCollisionOvershoot;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
						if(bFingerCollisionDebugTracing)
						{
							DrawDebugPoint(GetWorld(), Hit.Location, 20, FColor::Red, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY);
						}
#endif
					}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
					if(bFingerCollisionDebugTracing)
					{
						DrawDebugPoint(GetWorld(), Hit.Location, 13, FColor::Black, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY);
					}
#endif

					break;
				}

				FingerTraceLength += FVector::Distance(TraceStart, TraceEnd);
			}
		}

		EFingers namedFinger = static_cast<EFingers>(fingerIndex);
		FingerCollisionRangePercentages[namedFinger] = FMath::FInterpTo(FingerCollisionRangePercentages[namedFinger], FingerCollisionRangeTargetPercentages[fingerIndex], DeltaSeconds, FingerInterpSpeed);
	}

	PositionFingerCollisions();
}

TMap<TEnumAsByte<EFingers>, float> AVRPlayerHand::GetFingerRangePercentages()
{
	return FingerCollisionRangePercentages;
}

void AVRPlayerHand::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Get rid of strings and let then set in editor
	ThumbCollisionPosition->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform,  "thumb_02_r");
	IndexCollisionPosition->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform,  "index_01_r");
	MiddleCollisionPosition->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform,  "middle_01_r");
	RingCollisionPosition->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform,  "ring_01_r");
	LittleCollisionPosition->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform,  "pinky_01_r");

	for (int fingerIndex = 0; fingerIndex < 5; ++fingerIndex)
	{
		TArray<FVector, TInlineAllocator<5>> CurrentFingerCollisionPositions;

		for (int timePositionIndex = 0; timePositionIndex < 5; ++timePositionIndex)
		{
			const float CurCurveTime = FMath::Clamp(0.25f * timePositionIndex, 0.0f, 1.0f);
			CurrentFingerCollisionPositions.Add(SplineCollisionComponents[fingerIndex]->GetLocationAtTime(CurCurveTime, ESplineCoordinateSpace::Local));

			if(timePositionIndex > 0)
			{
				FingerSplineTotalRanges[fingerIndex] += FVector::Distance(CurrentFingerCollisionPositions[timePositionIndex], CurrentFingerCollisionPositions[timePositionIndex-1]);
			}
		}
		FingerCollisionPositions.Add(CurrentFingerCollisionPositions);
	}
}

void AVRPlayerHand::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MotionControllerComponent->bRight = bRightHand;
}

void AVRPlayerHand::PositionFingerCollisions() const
{
#define POS_FINGER_COLLISION(FingerName)																\
	FingerName##Collision->SetWorldLocation(##FingerName##CollisionPosition->GetComponentLocation());	\
	FingerName##Collision->SetWorldRotation(##FingerName##CollisionPosition->GetComponentRotation());	\
	FingerName##Collision->SetRelativeScale3D(FVector(1.0f, 1.0f, FMath::GetMappedRangeValueClamped(	\
		TRange<float>(0.0f, 1.0f),																		\
		TRange<float>(1.0f, 0.5),																		\
		MotionControllerComponent->FingerRangePercentages[EFingers::FingerName])));

	POS_FINGER_COLLISION(Thumb)
	POS_FINGER_COLLISION(Index)
	POS_FINGER_COLLISION(Middle)
	POS_FINGER_COLLISION(Ring)
	POS_FINGER_COLLISION(Little)
#undef POS_FINGER_COLLISION
}