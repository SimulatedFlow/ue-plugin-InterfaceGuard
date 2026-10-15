// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Editor module of InterfaceGuard.
 *
 * Adds exactly one thing the Runtime module cannot do on its own: finding every Blueprint asset
 * in the project that implements the watched interface, loaded or not, through the Asset
 * Registry, and applying the same UInterfaceGuardStatics::HasBlueprintOverride check used at
 * runtime to each one's GeneratedClass. Purely an optional convenience scan - the world scan
 * that ships in the Runtime module already works with no editor and no Asset Registry at all.
 */
class FInterfaceGuardEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Walks every Blueprint asset under /Game whose GeneratedClass implements
	 * IInterfaceGuardDemoInteractable, checks it for a Blueprint override of FunctionName, logs
	 * every finding, and folds each one into the active editor world's UInterfaceGuardSubsystem
	 * (if one exists) so the HUD panel reflects the project-wide result too.
	 *
	 * Reports only. No asset is ever loaded for anything beyond reading its GeneratedClass, and
	 * nothing is ever written back to an asset.
	 */
	static void ScanProjectBlueprints(FName FunctionName);

private:
	void RegisterMenus();
};
