// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardSubsystem.h"

#include "EngineUtils.h"
#include "InterfaceGuardHUD.h"
#include "InterfaceGuardInteractable.h"
#include "InterfaceGuardLog.h"
#include "InterfaceGuardStatics.h"
#include "Engine/World.h"
#include "UObject/Class.h"

#if WITH_EDITOR
#include "Debug/DebugDrawService.h"
#endif

UInterfaceGuardSubsystem* UInterfaceGuardSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	return World != nullptr ? World->GetSubsystem<UInterfaceGuardSubsystem>() : nullptr;
}

bool UInterfaceGuardSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	// Wider than the UWorldSubsystem default (Game/PIE only). Editor is added so this subsystem -
	// and therefore the panel's data - also exists while scrubbing the demo map in the Editor
	// viewport with no PIE running at all, which is exactly how the store video's camera flythrough
	// is captured (see PLUGIN-FERTIGSTELLER.md).
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::GamePreview
		|| WorldType == EWorldType::Editor;
}

void UInterfaceGuardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	EditorDrawHandle = UDebugDrawService::Register(
		TEXT("Game"),
		FDebugDrawDelegate::CreateUObject(this, &UInterfaceGuardSubsystem::OnEditorViewportDraw));
#endif
}

void UInterfaceGuardSubsystem::Deinitialize()
{
#if WITH_EDITOR
	if (EditorDrawHandle.IsValid())
	{
		UDebugDrawService::Unregister(EditorDrawHandle);
		EditorDrawHandle.Reset();
	}
#endif

	Super::Deinitialize();
}

void UInterfaceGuardSubsystem::ScanWorldForOverrides(FName FunctionName)
{
	Findings.Reset();
	bHasScannedAtLeastOnce = true;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogInterfaceGuard, Warning, TEXT("InterfaceGuard: ScanWorldForOverrides has no world to scan."));
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor == nullptr || !Actor->GetClass()->ImplementsInterface(UInterfaceGuardDemoInteractable::StaticClass()))
		{
			continue;
		}

		FString Reason;
		if (UInterfaceGuardStatics::HasBlueprintOverride(Actor->GetClass(), FunctionName, Reason))
		{
			FInterfaceGuardFinding Finding;
			Finding.ClassName = Actor->GetClass()->GetFName();
			Finding.FunctionName = FunctionName;
			Finding.Reason = Reason;
			Findings.Add(MoveTemp(Finding));
		}
	}

	UE_LOG(LogInterfaceGuard, Display, TEXT("InterfaceGuard world scan: %d override(s) found for %s"),
		Findings.Num(), *FunctionName.ToString());
}

void UInterfaceGuardSubsystem::ReportSafeCall(bool bHitBlueprintOverride)
{
	++NumSafeCalls;
	if (bHitBlueprintOverride)
	{
		++NumBlueprintPathHits;
	}
}

void UInterfaceGuardSubsystem::ReportDirectCall()
{
	++NumDirectCalls;
}

void UInterfaceGuardSubsystem::AddExternalFinding(const FInterfaceGuardFinding& Finding)
{
	for (const FInterfaceGuardFinding& Existing : Findings)
	{
		if (Existing.ClassName == Finding.ClassName && Existing.FunctionName == Finding.FunctionName)
		{
			return;
		}
	}

	Findings.Add(Finding);
	bHasScannedAtLeastOnce = true;
}

FString UInterfaceGuardSubsystem::SummaryLine() const
{
	return FString::Printf(
		TEXT("Overrides found: %d | Safe calls (Execute_): %d -> Blueprint hits: %d | Direct calls (wrong way): %d"),
		Findings.Num(), NumSafeCalls, NumBlueprintPathHits, NumDirectCalls);
}

void UInterfaceGuardSubsystem::DumpToLog() const
{
	UE_LOG(LogInterfaceGuard, Display, TEXT("%s"), *SummaryLine());

	if (Findings.Num() == 0)
	{
		UE_LOG(LogInterfaceGuard, Display,
			TEXT("  no Blueprint override found - run InterfaceGuard.Scan first if this looks wrong"));
		return;
	}

	for (const FInterfaceGuardFinding& Finding : Findings)
	{
		UE_LOG(LogInterfaceGuard, Display, TEXT("  %s::%s -> %s"),
			*Finding.ClassName.ToString(), *Finding.FunctionName.ToString(), *Finding.Reason);
	}
}

#if WITH_EDITOR
void UInterfaceGuardSubsystem::OnEditorViewportDraw(UCanvas* Canvas, APlayerController* PlayerController)
{
	// Only in an editor viewport that is NOT running PIE - a PIE viewport already gets the panel
	// through AInterfaceGuardHUDActor::DrawHUD, and drawing it twice would double the text.
	const UWorld* World = GetWorld();
	if (Canvas == nullptr || World == nullptr || World->IsGameWorld())
	{
		return;
	}

	AInterfaceGuardHUDActor::DrawPanelStatic(Canvas, this, FVector2D(24.f, 24.f), 900.f);
}
#endif
