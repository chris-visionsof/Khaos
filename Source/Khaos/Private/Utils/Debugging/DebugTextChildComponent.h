// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DebugTextChildComponent.generated.h"

enum EHorizTextAligment : int;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextChildDestroyedSignature, class AVRDebugText*, DestroyedText );

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KHAOS_API UDebugTextChildComponent : public UChildActorComponent 
{
	GENERATED_BODY()

public:
	UDebugTextChildComponent();

	virtual TObjectPtr<AVRDebugText> GetChildTextActor();

	virtual void SetHorizontalAlignment(const EHorizTextAligment& Alignment);

	virtual void SetLifeSpan(const float& LifeSpan);

	virtual void SetOwner(const TObjectPtr<AActor>& NewOwner);

	virtual void SetPositionPadding(const float& Padding);

	virtual void SetShouldRotate(const bool& InRotate);

	virtual void SetText(const FText& Text);

	virtual void SetTextRenderColor(const FColor& Color);

	virtual void SetWorldSize(const float& Size);
	
	FTextChildDestroyedSignature OnTextChildDestroyed;

protected:
	virtual void OnRegister() override;

	UFUNCTION()
	void OnTextDestroyed(AActor* DestroyedActor);
	
	
	TObjectPtr<AVRDebugText> ChildTextActor;
};
