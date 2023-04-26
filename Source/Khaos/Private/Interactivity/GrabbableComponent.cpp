// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableComponent.h"

#include "ContentStreaming.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Khaos/KhaosShowFlags.h"
#include "Player/VRPlayerPawn.h"

namespace
{
	bool OwnerLevelHasRegisteredStaticComponentsInStreamingManager(const AActor* Owner)
	{
		if (Owner)
		{
			const ULevel* Level = Owner->GetLevel();
			if (Level)
			{
				return Level->bStaticComponentsRegisteredInStreamingManager;
			}
		}
		return false;
	}
}

UGrabbableComponent::UGrabbableComponent()
{
	CanCharacterStepUpOn = ECB_No;
	Mobility = EComponentMobility::Movable;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionProfileName("Interactivity");
	SetCollisionResponseToAllChannels(ECR_Overlap);

	BodyInstance.bAutoWeld = true;
	bHiddenInGame = true;

	PrimaryComponentTick.bCanEverTick  = true;
}

void UGrabbableComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	// Make sure cached cull distance is up-to-date if its zero and we have an LD cull distance
	if( CachedMaxDrawDistance == 0.f && LDMaxDrawDistance > 0.f )
	{
		bool bNeverCull = bNeverDistanceCull || GetLODParentPrimitive();
		CachedMaxDrawDistance = bNeverCull ? 0.f : LDMaxDrawDistance;
	}

	UActorComponent::CreateRenderState_Concurrent(Context);

	UpdateBounds();

	// If the primitive isn't hidden and the detail mode setting allows it, add it to the scene.
	// VISIONS MOD
	if (bInteractivityPreviewCache || ShouldComponentAddToScene())
	// VISIONS MOD END
	{
		if (Context != nullptr)
		{
			Context->AddPrimitive(this);
		}
		else
		{
			GetWorld()->Scene->AddPrimitive(this);
		}
	}

	// *********** FROM PARENT ******************
	// Components are either registered as static or dynamic in the streaming manager.
	// Static components are registered in batches the first frame the level becomes visible (or incrementally each frame when loaded but not yet visible). 
	// The level static streaming data is never updated after this, and gets reused whenever the level becomes visible again (after being hidden).
	// Dynamic components, on the other hand, are updated whenever their render states change.
	// The following logic handles all cases where static components should fallback on the dynamic path.
	// It is based on a design where each component must either have bHandledByStreamingManagerAsDynamic or bAttachedToStreamingManagerAsStatic set.
	// If this is not the case, then the component has never been handled before.
	// The bIgnoreStreamingManagerUpdate flag is used to prevent handling component that are already in the update list or that don't have streaming data.
	if (!bIgnoreStreamingManagerUpdate && (Mobility != EComponentMobility::Static || bHandledByStreamingManagerAsDynamic || (!bAttachedToStreamingManagerAsStatic && OwnerLevelHasRegisteredStaticComponentsInStreamingManager(GetOwner()))))
	{
		FStreamingManagerCollection* Collection = IStreamingManager::Get_Concurrent();
		if (Collection)
		{
			Collection->NotifyPrimitiveUpdated_Concurrent(this);
		}
	}
}

void UGrabbableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UBodySetup* UGrabbableComponent::GetBodySetup()
{
	if (!IsValid(BodySetup))
	{
		BodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);
		BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		BodySetup->bMeshCollideAll = true;
		BodySetup->bNeverNeedsCookedCollisionData = true;

		CompleteBodySetup(BodySetup);

		BodyInstance.BodySetup = BodySetup;
	}
	
	UpdateBodySetup();
	return BodySetup;
}

FPrimitiveSceneProxy* UGrabbableComponent::CreateSceneProxy()
{
	return  new FGrabbableSceneProxy(this);
}

void UGrabbableComponent::OnRegister()
{
	if(!IsRegistered())
	{
		RenderCheck();

		// There is nothing to trigger the change of the show flags so we need to setup a timer to check
#if WITH_EDITOR
		if (GEditor)
		{
			GEditor->GetTimerManager()->SetTimer(InteractivityTimerHandle, FTimerDelegate::CreateUObject(this, &UGrabbableComponent::RenderCheck), 1, true);
		}
		else
#endif //WITH_EDITOR
		{
			GetWorld()->GetTimerManager().SetTimer(InteractivityTimerHandle, FTimerDelegate::CreateUObject(this, &UGrabbableComponent::RenderCheck), 1, true);
		}
	}
	
	OnComponentBeginOverlap.AddUniqueDynamic(this, &UGrabbableComponent::OnBeginOverlap);
	OnComponentEndOverlap.AddUniqueDynamic(this, &UGrabbableComponent::OnEndOverlap);
	
	Super::OnRegister();
}

void UGrabbableComponent::OnUnregister()
{
	Super::OnUnregister();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

#if WITH_EDITOR
	if (GEditor)
	{
		GEditor->GetTimerManager()->ClearTimer(InteractivityTimerHandle);
	}
	else
#endif //WITH_EDITOR
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractivityTimerHandle);
	}
#endif
	
	OnComponentBeginOverlap.RemoveAll(this);
	OnComponentEndOverlap.RemoveAll(this);
}

void UGrabbableComponent::SetGrabState(EGrabState NewGrabState)
{
	GrabState = NewGrabState;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if(bInteractivityPreviewCache)
	{
		MarkRenderStateDirty();
	}
#endif
}

void UGrabbableComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(IsValid(OtherActor) && OtherActor->IsA(AVRPlayerHand::StaticClass()))
	{
		SetGrabState(EGrabState::Grabbable);
	}
}

void UGrabbableComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(IsValid(OtherActor) && OtherActor->IsA(AVRPlayerHand::StaticClass()))
	{
		SetGrabState(EGrabState::Idle);
	}
}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
void UGrabbableComponent::RenderCheck()
{
	const bool bInteractivityEnabled = IsInteractivityEnabled();

	if(bInteractivityPreviewCache != bInteractivityEnabled)
	{
		MarkRenderStateDirty();
	}

	bInteractivityPreviewCache = bInteractivityEnabled;
}
#endif 

SIZE_T UGrabbableComponent::FGrabbableSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

uint32 UGrabbableComponent::FGrabbableSceneProxy::GetMemoryFootprint() const
{
	return(sizeof(*this) + GetAllocatedSize());
}


bool UGrabbableComponent::IsInteractivityEnabled() const
{
	bool bShowInteractivity;

	const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());

#if WITH_EDITOR
	if (GEditor && WorldContext && WorldContext->WorldType != EWorldType::Game)
	{
		bShowInteractivity = WorldContext->GameViewport != nullptr && FKhaosShowFlags::Interactivity.IsEnabled(WorldContext->GameViewport->EngineShowFlags);
		if (bShowInteractivity == false)
		{
			// we have to check all viewports because we can't to distinguish between SIE and PIE at this point.
			for (const FEditorViewportClient* CurrentViewport : GEditor->GetAllViewportClients())
			{
				if (CurrentViewport && FKhaosShowFlags::Interactivity.IsEnabled(CurrentViewport->EngineShowFlags))
				{
					bShowInteractivity = true;
					break;
				}
			}
		}
	}
	else
#endif WITH_EDITOR
	{
		bShowInteractivity = WorldContext && WorldContext->GameViewport && FKhaosShowFlags::Interactivity.IsEnabled(WorldContext->GameViewport->EngineShowFlags);
	}

	return bShowInteractivity;
}

void UGrabbableComponent::UpdateCollisionProfile()
{
	if(bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		UpdateBodySetup();
		CreatePhysicsState();

		BodySetup->InvalidatePhysicsData();
		BodySetup->CreatePhysicsMeshes();
	}
}

UGrabbableComponent::FGrabbableSceneProxy::FGrabbableSceneProxy(UGrabbableComponent* InComponent)
	:FPrimitiveSceneProxy(InComponent)
	,bInteractivityPreview(InComponent->bInteractivityPreviewCache)
{
	RepresentativeComponent = InComponent;
}

FPrimitiveViewRelevance UGrabbableComponent::FGrabbableSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Relevance;
	Relevance.bDrawRelevance = bInteractivityPreview || IsShown(View);
	Relevance.bDynamicRelevance = true;
	Relevance.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	Relevance.bVelocityRelevance = IsMovable();
	return Relevance;
}

void UGrabbableComponent::FGrabbableSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
	const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	const FVector CompLocation = RepresentativeComponent->GetComponentLocation();
	const FVector CompRelScale = RepresentativeComponent->GetRelativeScale3D();
	
	// TODO: Clean up and finish
	const FLinearColor Color = (RepresentativeComponent->GrabState == EGrabState::Grabbable) ? FLinearColor::Yellow : FLinearColor::Red;
	
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			if(InView(CompLocation, CompRelScale, View))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				switch (RepresentativeComponent->GrabbableBounds)
				{
				case EGrabbableBounds::Sphere:
					DrawWireSphere(PDI, CompLocation, Color, CompRelScale.X, 16, SDPG_MAX);
					break;
				case EGrabbableBounds::Box:
					break;
				case EGrabbableBounds::Capsule: break;
				default:;
				}
			}
		}
	}
}


