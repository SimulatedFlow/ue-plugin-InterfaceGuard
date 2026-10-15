// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardDemoGameMode.h"

#include "InterfaceGuardDemoPlayerController.h"
#include "InterfaceGuardHUD.h"
#include "InterfaceGuardSettings.h"
#include "InterfaceGuardSubsystem.h"

AInterfaceGuardDemoGameMode::AInterfaceGuardDemoGameMode()
{
	HUDClass = AInterfaceGuardHUDActor::StaticClass();
	PlayerControllerClass = AInterfaceGuardDemoPlayerController::StaticClass();
}

void AInterfaceGuardDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UInterfaceGuardSettings::Get()->bScanOnBeginPlay)
	{
		if (UInterfaceGuardSubsystem* Subsystem = UInterfaceGuardSubsystem::Get(this))
		{
			Subsystem->ScanWorldForOverrides(UInterfaceGuardSettings::Get()->WatchedFunctionName);
		}
	}
}
