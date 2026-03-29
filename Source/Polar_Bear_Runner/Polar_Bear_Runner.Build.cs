// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Polar_Bear_Runner : ModuleRules
{
	public Polar_Bear_Runner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Polar_Bear_Runner",
			"Polar_Bear_Runner/Variant_Platforming",
			"Polar_Bear_Runner/Variant_Platforming/Animation",
			"Polar_Bear_Runner/Variant_Combat",
			"Polar_Bear_Runner/Variant_Combat/AI",
			"Polar_Bear_Runner/Variant_Combat/Animation",
			"Polar_Bear_Runner/Variant_Combat/Gameplay",
			"Polar_Bear_Runner/Variant_Combat/Interfaces",
			"Polar_Bear_Runner/Variant_Combat/UI",
			"Polar_Bear_Runner/Variant_SideScrolling",
			"Polar_Bear_Runner/Variant_SideScrolling/AI",
			"Polar_Bear_Runner/Variant_SideScrolling/Gameplay",
			"Polar_Bear_Runner/Variant_SideScrolling/Interfaces",
			"Polar_Bear_Runner/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
