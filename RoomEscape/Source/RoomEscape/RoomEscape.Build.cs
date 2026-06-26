using UnrealBuildTool;

public class RoomEscape : ModuleRules
{
	public RoomEscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
