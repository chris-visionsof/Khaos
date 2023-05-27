// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugTextChildComponent.h"

#include "VRDebugText.h"



// Sets default values for this component's properties
UDebugTextChildComponent::UDebugTextChildComponent()
{
	SetChildActorClass(AVRDebugText::StaticClass());
}

TObjectPtr<AVRDebugText> UDebugTextChildComponent::GetChildTextActor()
{
	return ChildTextActor;
}

void UDebugTextChildComponent::SetHorizontalAlignment(const EHorizTextAligment& Alignment)
{
	ChildTextActor->SetHorizontalAlignment(Alignment);
}

void UDebugTextChildComponent::SetLifeSpan(const float& LifeSpan)
{
	ChildTextActor->SetLifeSpan(LifeSpan);
}

void UDebugTextChildComponent::SetOwner(const TObjectPtr<AActor>& NewOwner)
{
	ChildTextActor->SetOwner(NewOwner);
}

void UDebugTextChildComponent::SetShouldRotate(const bool& InRotate)
{
	ChildTextActor->SetShouldRotate(InRotate);
}

void UDebugTextChildComponent::SetText(const FText& Text)
{
	ChildTextActor->SetText(Text);
}

void UDebugTextChildComponent::SetWorldSize(const float& Size)
{
	ChildTextActor->SetWorldSize(Size);
}

void UDebugTextChildComponent::SetPositionPadding(const float& Padding)
{
	ChildTextActor->SetPositionPadding(Padding);
}


void UDebugTextChildComponent::OnRegister()
{
	Super::OnRegister();

	ChildTextActor = Cast<AVRDebugText>(GetChildActor());
	
	ChildTextActor->OnDestroyed.AddDynamic(this, &UDebugTextChildComponent::OnTextDestroyed);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UDebugTextChildComponent::OnTextDestroyed(AActor*)
{
	if(OnTextChildDestroyed.IsBound())
	{
		OnTextChildDestroyed.Broadcast(ChildTextActor);
	}
}

void UDebugTextChildComponent::SetTextRenderColor(const FColor& Color)
{
	GetChildTextActor()->SetTextRenderColor(Color); 
}
