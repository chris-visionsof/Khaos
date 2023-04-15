// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Khaos : ModuleRules
{
	public Khaos(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new []
		{
			"Core", 
			"CoreUObject", 
			"Engine",
			"EnhancedInput",
			"HeadMountedDisplay",
			"InputCore",
			"OpenXR",
			"OpenXRHMD",
			"OpenXRHandTracking",
			"PhysicsCore",
			"UnrealEd"
		});
	}
}
