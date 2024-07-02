// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class YRPG : ModuleRules
{
	public YRPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "AnimGraphRuntime" ,"AIModule","OnlineSubsystemSteam","OnlineSubsystem"});
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "OnlineSubsystem" });
	}
}
