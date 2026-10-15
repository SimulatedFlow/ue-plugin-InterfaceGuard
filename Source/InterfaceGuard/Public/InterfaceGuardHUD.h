// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InterfaceGuardHUD.generated.h"

class UCanvas;
class UInterfaceGuardSubsystem;

/**
 * The status panel. Drawn on UCanvas, no widget, no material, no asset - survives a cooked
 * Shipping build. Shows the live scan count and both call counters side by side, which is the
 * one thing a still frame needs to prove: the direct call and the Execute_ call produce a
 * different, visible result.
 */
UCLASS()
class INTERFACEGUARD_API AInterfaceGuardHUDActor : public AHUD
{
	GENERATED_BODY()

public:
	AInterfaceGuardHUDActor();

	virtual void DrawHUD() override;

	/** Instance entry point - resolves this HUD's subsystem and draws through DrawPanelStatic. */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	void DrawPanel(UCanvas* InCanvas, const FVector2D& Origin, float Width);

	/**
	 * The actual drawing, with no AHUD instance required - shared between DrawHUD (the PIE / -game
	 * path) and UInterfaceGuardSubsystem::OnEditorViewportDraw (the editor-viewport-without-PIE
	 * path), so there is exactly one truth about what the panel looks like.
	 */
	static void DrawPanelStatic(UCanvas* InCanvas, const UInterfaceGuardSubsystem* Subsystem, const FVector2D& Origin, float Width);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InterfaceGuard")
	FVector2D PanelOrigin = FVector2D(24.f, 24.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InterfaceGuard")
	float PanelWidth = 900.f;
};
