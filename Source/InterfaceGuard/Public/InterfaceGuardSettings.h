// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InterfaceGuardSettings.generated.h"

/**
 * Project defaults, under Project Settings > Plugins > InterfaceGuard.
 *
 * These live in config rather than in an actor so a build server invocation
 * (InterfaceGuard.Scan with no argument, or the editor's project-wide scan) can run against the
 * same watched function a project has already agreed on, with nothing hand-typed on the command
 * line.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "InterfaceGuard"))
class INTERFACEGUARD_API UInterfaceGuardSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Convenience accessor. Never null. */
	static const UInterfaceGuardSettings* Get() { return GetDefault<UInterfaceGuardSettings>(); }

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/**
	 * The interface function InterfaceGuard.Scan and the editor's project-wide scan check when no
	 * function name is given explicitly.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "InterfaceGuard")
	FName WatchedFunctionName = TEXT("OnInteract");

	/**
	 * Scan automatically when the demo's game mode starts.
	 *
	 * On by default so a store screenshot of the demo map, taken with no click at all, already
	 * shows a real "Overrides found: 1" rather than an empty, unconvincing zero.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "InterfaceGuard")
	bool bScanOnBeginPlay = true;

	/**
	 * Draw the on-screen panel.
	 *
	 * On by default - unlike an always-on debug overlay, this panel is also the thing the demo
	 * exists to show, not just a developer convenience.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "InterfaceGuard")
	bool bShowOverlay = true;
};
