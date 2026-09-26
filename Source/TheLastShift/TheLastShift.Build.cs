using UnrealBuildTool;

public class TheLastShift : ModuleRules
{
	public TheLastShift(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput", "AIModule", "UMG", "Slate", "SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}

