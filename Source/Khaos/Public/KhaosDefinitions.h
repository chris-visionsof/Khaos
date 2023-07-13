#pragma once

#include "KhaosDefinitions.generated.h"

//TODO: Add custom property editors for custom FNames in the editor
namespace CollisionProfiles 
{
	const FName PawnBlockAllButSelf("PawnBlockAllButSelf");
	const FName InteractiveObj("InteractiveObj");
	const FName InteractiveTrigger("InteractiveTrigger");
}

namespace ActorTags 
{
	const FName Grabbable("Grabbable");
	const FName Actionable("Actionable");
}

UCLASS()
class KHAOS_API UKhaosDefinitionsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Actor Tags")
	static FName GetGrabbableTag()	{ return ActorTags::Grabbable; }
};
