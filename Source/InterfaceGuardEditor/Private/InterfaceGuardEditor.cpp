// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardEditor.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "InterfaceGuardInteractable.h"
#include "InterfaceGuardLog.h"
#include "InterfaceGuardSettings.h"
#include "InterfaceGuardStatics.h"
#include "InterfaceGuardSubsystem.h"
#include "InterfaceGuardTypes.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "UObject/Class.h"

#define LOCTEXT_NAMESPACE "FInterfaceGuardEditorModule"

void FInterfaceGuardEditorModule::StartupModule()
{
	// Menu and tab registration waits for the ToolMenus startup callback, which is not ready yet
	// at Default loading phase - and never matters for a commandlet anyway.
	if (IsRunningCommandlet())
	{
		return;
	}

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FInterfaceGuardEditorModule::RegisterMenus));
}

void FInterfaceGuardEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FInterfaceGuardEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	if (UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")))
	{
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection(
			TEXT("InterfaceGuard"),
			LOCTEXT("InterfaceGuardSection", "Interface Guard"));

		Section.AddMenuEntry(
			TEXT("InterfaceGuardScanProject"),
			LOCTEXT("ScanProjectLabel", "Scan Project For Interface Overrides"),
			LOCTEXT("ScanProjectTooltip",
				"Finds every Blueprint asset under /Game that overrides the watched interface function - "
				"the classes a direct C++ call would silently skip. Reports only; never changes an asset."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Search")),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FInterfaceGuardEditorModule::ScanProjectBlueprints(UInterfaceGuardSettings::Get()->WatchedFunctionName);
			})));
	}
}

void FInterfaceGuardEditorModule::ScanProjectBlueprints(FName FunctionName)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName(TEXT("/Game")));
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);

	// The active editor world, so a project-wide scan also updates the HUD panel if the demo map
	// happens to be open - a convenience, not a requirement; the scan logs regardless.
	UWorld* EditorWorld = (GEditor != nullptr) ? GEditor->GetEditorWorldContext().World() : nullptr;
	UInterfaceGuardSubsystem* Subsystem = (EditorWorld != nullptr) ? EditorWorld->GetSubsystem<UInterfaceGuardSubsystem>() : nullptr;

	int32 NumFound = 0;

	for (const FAssetData& Asset : Assets)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset());
		if (Blueprint == nullptr || Blueprint->GeneratedClass == nullptr)
		{
			continue;
		}

		if (!Blueprint->GeneratedClass->ImplementsInterface(UInterfaceGuardDemoInteractable::StaticClass()))
		{
			continue;
		}

		FString Reason;
		if (!UInterfaceGuardStatics::HasBlueprintOverride(Blueprint->GeneratedClass, FunctionName, Reason))
		{
			continue;
		}

		++NumFound;
		UE_LOG(LogInterfaceGuard, Display, TEXT("InterfaceGuard project scan: %s -> %s"),
			*Blueprint->GeneratedClass->GetName(), *Reason);

		if (Subsystem != nullptr)
		{
			FInterfaceGuardFinding Finding;
			Finding.ClassName = Blueprint->GeneratedClass->GetFName();
			Finding.FunctionName = FunctionName;
			Finding.Reason = Reason;
			Subsystem->AddExternalFinding(Finding);
		}
	}

	UE_LOG(LogInterfaceGuard, Display,
		TEXT("InterfaceGuard project scan complete: %d Blueprint override(s) of %s found under /Game"),
		NumFound, *FunctionName.ToString());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInterfaceGuardEditorModule, InterfaceGuardEditor)
