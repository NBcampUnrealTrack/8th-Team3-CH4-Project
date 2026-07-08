using UnrealBuildTool;

public class RoomEscape : ModuleRules
{
	public RoomEscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// C++ 클래스 생성 시 include 경로 오류 방지를 위한 Path 추가
		PublicIncludePaths.AddRange(new string[] { "RoomEscape" });

        // C++ 클래스 생성 시 include 경로 오류 방지를 위한 Path 추가
        PrivateIncludePaths.AddRange(new string[] { "RoomEscape" });
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			// for UI
			"UMG",
			"CommonUI",
			"CommonInput",
			// Add Custom Implement Modules
			"WidgetUtility",
			// GameplayAbilities
			"GameplayAbilities", "GameplayTasks", "GameplayTags",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			// for UI
            "Slate",
            "SlateCore",
			// Add Custom Implement Modules
			"WidgetUtility",
        });
	}
}
