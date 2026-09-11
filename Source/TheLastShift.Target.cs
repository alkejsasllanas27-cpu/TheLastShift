using UnrealBuildTool;
using System.Collections.Generic;

public class TheLastShiftTarget : TargetRules
{
	public TheLastShiftTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("TheLastShift");
	}
}
