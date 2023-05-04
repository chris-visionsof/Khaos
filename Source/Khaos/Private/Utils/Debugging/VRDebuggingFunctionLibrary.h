// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VRDebuggingFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class KHAOS_API UVRDebuggingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="VR|Debug", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void DisplayDebugText(const UObject* WorldContextObject, FText DebugText);
};
