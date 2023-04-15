// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRDebugTextCollectionActor.generated.h"


UCLASS()
class KHAOS_API AVRDebugTextCollectionActor : public AActor
{
	GENERATED_BODY()

public:
	AVRDebugTextCollectionActor();

	virtual const AVRDebugText* DisplayDebugText(const FText& DebugMessage, const float& TimeToDisplay=5.0f, const FColor& DisplayColor=FColor::Red, const float& Scale=1.0f);
	
	virtual void SetGeneralWorldSize(const float& Size) { GeneralWorldSize = std::move(Size); };
	
protected:
	UFUNCTION(BlueprintCallable, DisplayName="Display Debug Text", Category="VR|Debug",  meta=(AdvancedDisplay=1))
    virtual const AVRDebugText* DisplayDebugText_BP(FText DebugMessage, float TimeToDisplay=5.0f, FColor DisplayColor=FColor::Red, float Scale=1.0f);
	
	UFUNCTION(BlueprintCallable, DisplayName="Set General World Size", Category="VR|Debug", meta=(AdvancedDisplay=1))
    virtual void SetGeneralWorldSize_BP(float Size) ;
	
	UFUNCTION()
	void OnTextDestroyed(AVRDebugText* DestroyedActor);

	TMap<TObjectPtr<AVRDebugText>, TObjectPtr<class UDebugTextChildComponent>> DebugTextActors;

	UPROPERTY()
	float GeneralWorldSize = 1.0f;
};
