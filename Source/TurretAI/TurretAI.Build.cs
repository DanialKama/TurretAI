// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TurretAI : ModuleRules
{
	public TurretAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"NetCore",
			"Niagara"
		});
	}
}
