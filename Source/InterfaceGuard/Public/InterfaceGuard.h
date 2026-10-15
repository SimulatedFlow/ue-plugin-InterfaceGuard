// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Runtime module of InterfaceGuard.
 *
 * Holds the watched interface, the demo pedestal, the checker statics, the world subsystem with
 * the console commands, and the HUD panel. Nothing here needs the editor or the Asset Registry -
 * UFunction::Script.Num() is a plain reflection fact available on any loaded UClass, which is what
 * keeps the whole check usable in a cooked -game build, not just inside the editor.
 */
class FInterfaceGuardModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
