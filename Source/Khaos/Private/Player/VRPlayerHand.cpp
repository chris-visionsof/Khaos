#include "VRPlayerHand.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Interactivity/GrabbableComponent.h"

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
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AddInstanceComponent(SkeletalMeshComponent);

	GrabOverlapBox = CreateDefaultSubobject<UBoxComponent>("GrabOverlapBox");
	GrabOverlapBox->OnComponentBeginOverlap.AddDynamic(this, &AVRPlayerHand::OnGrabBoxBeginOverlap);
	GrabOverlapBox->OnComponentEndOverlap.AddDynamic(this, &AVRPlayerHand::OnGrabBoxEndOverlap);
	GrabOverlapBox->SetupAttachment(SkeletalMeshComponent);

	auto CollisionSplineSetup = [this](TObjectPtr<USplineComponent>& SplineComponentToSetup, FName Name)
	{
		SplineComponentToSetup = CreateDefaultSubobject<USplineComponent>(Name);
		SplineComponentToSetup->SetupAttachment(SkeletalMeshComponent);
		SplineComponentToSetup->Duration = 1.0f;
		SplineCollisionComponents.Add(SplineComponentToSetup);
	};

	CollisionSplineSetup(ThumbCollisionSpline, "ThumbCollisionSpline");
	CollisionSplineSetup(IndexCollisionSpline, "IndexCollisionSpline");
	CollisionSplineSetup(MiddleCollisionSpline, "MiddleCollisionSpline");
	CollisionSplineSetup(RingCollisionSpline, "RingCollisionSpline");
	CollisionSplineSetup(LittleCollisionSpline, "LittleCollisionSpline");
}

void AVRPlayerHand::OnPlayerGrabAction()
{
	if(OverlappingActor.IsSet())
	{
		OverlappingActor.GetValue().Value->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AVRPlayerHand::Tick(float DeltaSeconds)
{
	FingerCollisionRangeTargetPercentages = MotionControllerComponent->FingerRangePercentages;

	int FingersGrasped = 0;

	for (int fingerIndex = 0; fingerIndex < 5; ++fingerIndex)
	{
		TArray<FVector, TInlineAllocator<5>> CurrentFingerCollisionPositions = FingerCollisionPositions[fingerIndex];
		float MotionControlFingerPercentage = MotionControllerComponent->FingerRangePercentages[fingerIndex];

		if(bFingerCollisionDebugTracing || MotionControlFingerPercentage > 0.05)
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
				TraceParams.AddIgnoredActor(this);
				TraceParams.AddIgnoredActor(GetOwner());
				if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldDynamic, TraceParams))
				{
					FingerTraceLength += FVector::Distance(TraceStart, Hit.Location);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
					if(bFingerCollisionDebugTracing)
					{
						DrawDebugPoint(GetWorld(), Hit.Location, 15, FColor::Black, bPERSISTENT_LINES, DEBUG_LIFE_TIME, DEPTH_PRIORITY);
					}
#endif

					float TraceFingerPercentage = FingerTraceLength / FingerSplineTotalRanges[fingerIndex];
					if(MotionControlFingerPercentage > TraceFingerPercentage)
					{
						++FingersGrasped;

						FingerCollisionRangeTargetPercentages[fingerIndex] = TraceFingerPercentage + FingerCollisionOvershoot;
					}
					break;
				}

				FingerTraceLength += FVector::Distance(TraceStart, TraceEnd);
			}
		}

		EFingers namedFinger = static_cast<EFingers>(fingerIndex);
		FingerCollisionRangePercentages[namedFinger] = FMath::FInterpTo(FingerCollisionRangePercentages[namedFinger], FingerCollisionRangeTargetPercentages[fingerIndex], DeltaSeconds, FingerInterpSpeed);
	}
}

TMap<TEnumAsByte<EFingers>, float> AVRPlayerHand::GetFingerRangePercentages()
{
	return FingerCollisionRangePercentages;
}

void AVRPlayerHand::BeginPlay()
{
	Super::BeginPlay();

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

void AVRPlayerHand::OnGrabBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if(!IsValid(OtherActor)) { return; }

	if(OtherComp->IsA(UGrabbableComponent::StaticClass()))
	{
		UGrabbableComponent* GrabbableComp = Cast<UGrabbableComponent>(OtherComp);
		OverlappingActor = TTuple<TObjectPtr<UGrabbableComponent>, TObjectPtr<AActor>>(GrabbableComp, OtherActor);
	}
}

void AVRPlayerHand::OnGrabBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32)
{
	if(!IsValid(OtherActor) || !OverlappingActor.IsSet()) { return; }

	if(OtherComp->IsA(UGrabbableComponent::StaticClass())
		&& OtherActor == OverlappingActor.GetValue().Value.Get())
	{
		OverlappingActor.Reset();
	}
}

