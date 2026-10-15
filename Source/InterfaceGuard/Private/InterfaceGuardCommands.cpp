// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "InterfaceGuardLog.h"
#include "InterfaceGuardSettings.h"
#include "InterfaceGuardSubsystem.h"
#include "InterfaceGuardTypes.h"

// Registered as free console commands, not as member functions the subsystem has to register and
// unregister itself - one world, one subsystem instance, zero lifetime bookkeeping to get wrong.
namespace
{
	/** The actual game world, not whatever GEngine->GetWorld() happens to return in the editor. */
	UWorld* FindGameWorld(UWorld* Given)
	{
		if (Given != nullptr && Given->IsGameWorld())
		{
			return Given;
		}

		if (GEngine == nullptr)
		{
			return nullptr;
		}

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() != nullptr && Context.World()->IsGameWorld())
			{
				return Context.World();
			}
		}

		return nullptr;
	}

	UInterfaceGuardSubsystem* FindService(UWorld* World)
	{
		UWorld* GameWorld = FindGameWorld(World);
		return GameWorld != nullptr ? GameWorld->GetSubsystem<UInterfaceGuardSubsystem>() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs CmdScan(
		TEXT("InterfaceGuard.Scan"),
		TEXT("InterfaceGuard.Scan [FunctionName] - scans every actor implementing IInterfaceGuardDemoInteractable "
			 "for a Blueprint override of the named function (default: the watched function from Project Settings)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UInterfaceGuardSubsystem* Service = FindService(World);
				if (Service == nullptr)
				{
					UE_LOG(LogInterfaceGuard, Error, TEXT("InterfaceGuard.Scan: no game world, so nothing was scanned."));
					return;
				}

				const FName FunctionName = (Args.Num() > 0)
					? FName(*Args[0])
					: UInterfaceGuardSettings::Get()->WatchedFunctionName;

				Service->ScanWorldForOverrides(FunctionName);
				UE_LOG(LogInterfaceGuard, Display, TEXT("%s"), *Service->SummaryLine());
			}));

	FAutoConsoleCommandWithWorldAndArgs CmdDump(
		TEXT("InterfaceGuard.Dump"),
		TEXT("InterfaceGuard.Dump - prints the summary and every finding from the last scan to the log."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>&, UWorld* World)
			{
				if (UInterfaceGuardSubsystem* Service = FindService(World))
				{
					Service->DumpToLog();
				}
			}));

	FAutoConsoleCommandWithWorldAndArgs CmdShow(
		TEXT("InterfaceGuard.Show"),
		TEXT("InterfaceGuard.Show [0|1] - shows or hides the on-screen panel."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UInterfaceGuardSettings* Settings = GetMutableDefault<UInterfaceGuardSettings>();
				Settings->bShowOverlay = (Args.Num() > 0) ? (FCString::Atoi(*Args[0]) != 0) : true;
				UE_LOG(LogInterfaceGuard, Display, TEXT("InterfaceGuard.Show: panel is now %s."),
					Settings->bShowOverlay ? TEXT("on") : TEXT("off"));
			}));
}
