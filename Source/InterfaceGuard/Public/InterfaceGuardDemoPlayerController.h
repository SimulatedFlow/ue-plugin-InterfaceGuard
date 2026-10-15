// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InterfaceGuardDemoPlayerController.generated.h"

/**
 * Set Input Mode Game And UI + Show Mouse Cursor on BeginPlay - REQUIRED, not optional. On the
 * recording machine no mouse click reaches a UMG widget without this, and the keyboard does not
 * reach the game there at all (see PLUGIN-FERTIGSTELLER.md, "Die Tastatur erreicht das Spiel auf
 * dieser Maschine NICHT").
 */
UCLASS()
class INTERFACEGUARD_API AInterfaceGuardDemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
};
