// Fill out your copyright notice in the Description page of Project Settings.


#include "VRDebuggingFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Player/VR/VRPlayerPawn.h"

class AVRPlayerPawn;

namespace 
{
	static AVRPlayerPawn* VRPawn;
}

void UVRDebuggingFunctionLibrary::DisplayDebugText(const UObject* WorldContextObject, FText DebugText)
{
	if(!VRPawn)
	{
		if(const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if(const APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				VRPawn = Cast<AVRPlayerPawn>(PlayerController->GetPawnOrSpectator());
			}
		}
	}

	if(VRPawn)
	{
		VRPawn->DisplayDebugText(DebugText);
	}
}