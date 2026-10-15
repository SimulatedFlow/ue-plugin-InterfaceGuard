// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InterfaceGuardDemoGameMode.generated.h"

/**
 * The demo map's game mode. Sets the HUD and player controller classes, and runs the first scan
 * automatically on BeginPlay (if UInterfaceGuardSettings::bScanOnBeginPlay is on, which is the
 * default) - so a still frame of the map, taken with no click at all, already shows a real
 * "Overrides found: 1" rather than an unconvincing zero.
 */
UCLASS()
class INTERFACEGUARD_API AInterfaceGuardDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInterfaceGuardDemoGameMode();

	virtual void BeginPlay() override;
};
