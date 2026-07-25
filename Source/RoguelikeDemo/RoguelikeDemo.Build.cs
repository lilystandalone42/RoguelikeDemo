using UnrealBuildTool;

public class RoguelikeDemo : ModuleRules
{
	public RoguelikeDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"NavigationSystem",
			"AIModule",
			"Niagara",
			"EnhancedInput",
			"GameplayTags"
		});
	}
}
