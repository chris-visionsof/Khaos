// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRDebugText.generated.h"

enum EHorizTextAligment : int;

UCLASS()
class KHAOS_API AVRDebugText : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVRDebugText();
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="VR|Debug")
	virtual float GetWorldSize();

	UFUNCTION(BlueprintCallable, Category="VR|Debug")
    virtual void SetHorizontalAlignment(const EHorizTextAligment& Alignment);
	
	UFUNCTION(BlueprintCallable, meta=(DisplayName="Set Position Padding", ScriptName="SetPositionPadding"))
	void SetPositionPadding(const float& InPositionPadding) { PositionPadding = std::move(InPositionPadding); }

	UFUNCTION(BlueprintCallable, Category="VR|Debug")
	virtual void SetText(const FText& Text);
	
	UFUNCTION(BlueprintCallable, Category="VR|Debug")
	virtual void SetTextRenderColor(const FColor& Color);
	
	UFUNCTION(BlueprintCallable, Category="VR|Debug")
	virtual void SetWorldSize(const float& Size);

	UFUNCTION(BlueprintCallable, Category="VR|Debug")
	virtual void SetShouldRotate(const bool& bInRotate);
	
protected:
	UPROPERTY(EditAnywhere, DisplayName="Text Renderer", Category="VR|Debug")
	class UTextRenderComponent* DebugText;

	FVector UnobstructedPosition;
	FVector CachedHMDPos;
    FQuat CachedHMDRot;
    
	float WorldSizeCache;
	float PositionPadding = 0;
	
	bool bRotate = true;
};
