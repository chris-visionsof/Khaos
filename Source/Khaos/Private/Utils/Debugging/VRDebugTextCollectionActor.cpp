// Fill out your copyright notice in the Description page of Project Settings.

#include "VRDebugTextCollectionActor.h"
#include "DebugTextChildComponent.h"

#include "VRDebugText.h"
#include "Components/TextRenderComponent.h"

AVRDebugTextCollectionActor::AVRDebugTextCollectionActor()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

const AVRDebugText* AVRDebugTextCollectionActor::DisplayDebugText(const FText& DebugMessage, const float& TimeToDisplay, const FColor& DisplayColor, const float& Scale)
{
	const float WorldVerticalSize = Scale * GeneralWorldSize;
	
	const TObjectPtr<UDebugTextChildComponent> NewDebugText = NewObject<UDebugTextChildComponent>(this, FName(FString::Printf(TEXT("DebugText%s"), *FGuid::NewGuid().ToString())));
	NewDebugText->RegisterComponent();
	NewDebugText->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewDebugText->SetText(DebugMessage);
	NewDebugText->SetWorldSize(WorldVerticalSize);
	NewDebugText->SetTextRenderColor(DisplayColor);
	NewDebugText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	NewDebugText->SetLifeSpan(TimeToDisplay);
	NewDebugText->SetShouldRotate(false);
	NewDebugText->SetOwner(GetOwner());
	NewDebugText->OnTextChildDestroyed.AddDynamic(this, &AVRDebugTextCollectionActor::OnTextDestroyed);

	for (const TTuple<TObjectPtr<AVRDebugText>, TObjectPtr<UDebugTextChildComponent>>& CurTexActor : DebugTextActors)
	{
		CurTexActor.Value->AddRelativeLocation(FVector(0.0f, 0.0f, WorldVerticalSize));
	}
	
	DebugTextActors.Add(NewDebugText->GetChildTextActor(), NewDebugText);
	
	return NewDebugText->GetChildTextActor();
}

const AVRDebugText* AVRDebugTextCollectionActor::DisplayDebugText_BP(FText DebugMessage, float TimeToDisplay,
	FColor DisplayColor, float Scale)
{
	return DisplayDebugText(DebugMessage, TimeToDisplay, DisplayColor, Scale);
}

void AVRDebugTextCollectionActor::SetGeneralWorldSize_BP(float Size)
{
	SetGeneralWorldSize_BP(Size);
}

void AVRDebugTextCollectionActor::OnTextDestroyed(AVRDebugText* DestroyedActor)
{
	DebugTextActors.Remove(DestroyedActor);

	for (const TTuple<TObjectPtr<AVRDebugText>, TObjectPtr<UDebugTextChildComponent>>& CurTextActorPair : DebugTextActors)
	{
		const TObjectPtr<AVRDebugText> CurTextActor = CurTextActorPair.Key;
		if(CurTextActor->GetActorLocation().Z > DestroyedActor->GetActorLocation().Z)
		{
			CurTextActor->AddActorLocalOffset(FVector(0.0f, 0.0f, -DestroyedActor->GetWorldSize()));
		}
	}
}



