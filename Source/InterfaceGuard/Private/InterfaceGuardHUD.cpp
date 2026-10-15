// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "InterfaceGuardSettings.h"
#include "InterfaceGuardSubsystem.h"
#include "InterfaceGuardTypes.h"

AInterfaceGuardHUDActor::AInterfaceGuardHUDActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInterfaceGuardHUDActor::DrawHUD()
{
	Super::DrawHUD();

	if (Canvas != nullptr && UInterfaceGuardSettings::Get()->bShowOverlay)
	{
		DrawPanel(Canvas, PanelOrigin, PanelWidth);
	}
}

void AInterfaceGuardHUDActor::DrawPanel(UCanvas* InCanvas, const FVector2D& Origin, float Width)
{
	DrawPanelStatic(InCanvas, UInterfaceGuardSubsystem::Get(this), Origin, Width);
}

void AInterfaceGuardHUDActor::DrawPanelStatic(UCanvas* InCanvas, const UInterfaceGuardSubsystem* Subsystem, const FVector2D& Origin, float Width)
{
	if (InCanvas == nullptr || Subsystem == nullptr)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (Font == nullptr)
	{
		return;
	}

	const TArray<FInterfaceGuardFinding> Findings = Subsystem->GetFindings();
	const int32 NumRows = FMath::Max(1, Findings.Num());

	const float LineH = 15.f;
	const float Height = 8.f + LineH * (2.f + NumRows);

	FCanvasTileItem Box(Origin, FVector2D(Width, Height), FLinearColor(0.02f, 0.04f, 0.09f, 0.82f));
	Box.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(Box);

	float Y = Origin.Y + 6.f;
	const float X = Origin.X + 10.f;

	auto Write = [&](const FString& Text, const FLinearColor& Colour)
	{
		FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(Text), Font, Colour);
		Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.85f));
		InCanvas->DrawItem(Item);
		Y += LineH;
	};

	const FLinearColor White(0.86f, 0.92f, 0.98f);
	const FLinearColor Dim(0.62f, 0.70f, 0.78f);
	const FLinearColor Red(0.95f, 0.30f, 0.30f);

	Write(Subsystem->SummaryLine(), White);

	if (!Subsystem->HasScannedAnything())
	{
		Write(TEXT("not scanned yet - click Scan"), Dim);
		return;
	}

	if (Findings.Num() == 0)
	{
		Write(TEXT("no Blueprint override found for the watched function"), Dim);
		return;
	}

	for (const FInterfaceGuardFinding& Finding : Findings)
	{
		Write(FString::Printf(TEXT("%s::%s -> has Blueprint override, use Execute_"),
			*Finding.ClassName.ToString(), *Finding.FunctionName.ToString()), Red);
	}
}
