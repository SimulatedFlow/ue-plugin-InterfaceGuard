// Copyright 2026 Silvan Teufel. All Rights Reserved.

using UnrealBuildTool;

public class InterfaceGuard : ModuleRules
{
	public InterfaceGuard(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// The whole check - the interface, the demo pedestal, the statics, the subsystem and the
		// HUD panel - lives here, in a Runtime module, on purpose. HasBlueprintOverride is a pure
		// reflection question (UFunction::Script.Num() > 0) that needs no Asset Registry and no
		// editor, so the same check that drives the in-game HUD also runs in a cooked -game build.
		// The editor module next door adds exactly one thing this module cannot do on its own:
		// finding Blueprint assets that have never been loaded this session.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",

			// UWorldSubsystem, AActor, AHUD, AGameModeBase, APlayerController, UStaticMeshComponent,
			// UMaterialInstanceDynamic, TActorIterator, UDebugDrawService.
			"Engine",

			// UInterfaceGuardSettings is a UDeveloperSettings, so the watched function name and the
			// scan-on-BeginPlay switch appear under Project Settings > Plugins > InterfaceGuard with
			// no editor module involved.
			"DeveloperSettings",
		});

		// Deliberately NOT here:
		//   UnrealEd / AssetRegistry - finding Blueprint assets that are not currently loaded is the
		//                              editor module's business. UInterfaceGuardStatics::HasBlueprintOverride
		//                              takes a plain UClass*, so the whole check stays usable with no
		//                              editor, no world and no registry to stand up first.
		//   UMG                      - the demo's clickable buttons are a Widget Blueprint, authored
		//                              in the content phase. No C++ UUserWidget subclass exists, so
		//                              UMG is not a code dependency of this module.
	}
}
