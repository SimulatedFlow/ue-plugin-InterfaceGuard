// Copyright 2026 Silvan Teufel. All Rights Reserved.

using UnrealBuildTool;

public class InterfaceGuardEditor : ModuleRules
{
	public InterfaceGuardEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// The one thing the Runtime module cannot do on its own: finding every Blueprint asset in
		// the project that implements the watched interface, loaded or not, through the Asset
		// Registry, and applying the same HasBlueprintOverride check to each one's GeneratedClass.
		// Purely a convenience on top of the world scan that ships in the Runtime module and already
		// works with no editor at all.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",

			// The interface, the statics, the subsystem, the types and the settings.
			"InterfaceGuard",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Where the Blueprint assets to scan come from, loaded or not.
			"AssetRegistry",

			// GEditor, the editor world context, UToolMenus.
			"UnrealEd",

			// UBlueprint::GeneratedClass and the Blueprint compiler's own view of a class - kept as
			// a dependency even though this module's own code only touches GeneratedClass as a plain
			// UClass*, because the project-scan menu entry is exactly the kind of tool that grows a
			// "jump to the graph" feature next, and that feature lives here, not in Engine.
			"Kismet",

			// The entry under Tools.
			"ToolMenus",

			// FSlateIcon / FAppStyle for the menu entry's icon.
			"Slate",
			"SlateCore",
		});
	}
}
