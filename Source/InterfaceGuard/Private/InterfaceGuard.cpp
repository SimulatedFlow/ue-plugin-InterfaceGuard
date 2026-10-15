// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuard.h"
#include "InterfaceGuardLog.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FInterfaceGuardModule"

DEFINE_LOG_CATEGORY(LogInterfaceGuard);

void FInterfaceGuardModule::StartupModule()
{
	// Nothing to do. The console commands are free functions registered at static-init time
	// (InterfaceGuardCommands.cpp); the scan itself belongs to the world subsystem, which is where
	// a world exists to scan.
}

void FInterfaceGuardModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInterfaceGuardModule, InterfaceGuard)
