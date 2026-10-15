// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InterfaceGuardTypes.h"
#include "InterfaceGuardSubsystem.generated.h"

class APlayerController;
class UCanvas;

/**
 * Collects scan findings and the two live counters the HUD panel shows.
 *
 * A UWorldSubsystem rather than a game instance subsystem on purpose: ScanWorldForOverrides walks
 * the actors of one specific world (TActorIterator needs one), and DoesSupportWorldType is
 * widened beyond the engine default so this subsystem also exists for the Editor world - which is
 * what lets the panel draw in an editor viewport with no PIE running at all (see
 * OnEditorViewportDraw), for the store video's camera flythrough.
 */
UCLASS()
class INTERFACEGUARD_API UInterfaceGuardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UInterfaceGuardSubsystem* Get(const UObject* WorldContextObject);

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Scans every actor in this subsystem's world that implements IInterfaceGuardDemoInteractable,
	 * and replaces Findings with the classes that have a Blueprint override of FunctionName.
	 *
	 * An instantaneous reflection check, not an observation window - there is no need to wait and
	 * see, UFunction::Script is either populated or it is not.
	 */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	void ScanWorldForOverrides(FName FunctionName);

	/** Called by UInterfaceGuardStatics::CallInterfaceFunctionSafely after every Execute_-style call. */
	void ReportSafeCall(bool bHitBlueprintOverride);

	/** Called by UInterfaceGuardStatics::CallInterfaceFunctionDirectly_WRONG_WAY after every direct call. */
	void ReportDirectCall();

	/**
	 * Folds one externally-discovered finding in (from the editor module's project-wide Asset
	 * Registry scan), skipping it if a finding for the same class and function already exists.
	 */
	void AddExternalFinding(const FInterfaceGuardFinding& Finding);

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	TArray<FInterfaceGuardFinding> GetFindings() const { return Findings; }

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	int32 GetNumSafeCalls() const { return NumSafeCalls; }

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	int32 GetNumDirectCalls() const { return NumDirectCalls; }

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	int32 GetNumBlueprintPathHits() const { return NumBlueprintPathHits; }

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	bool HasScannedAnything() const { return bHasScannedAtLeastOnce; }

	/** One line for the HUD: "Overrides found: N | Safe calls (Execute_): X -> Blueprint hits: H | Direct calls (wrong way): Y". */
	UFUNCTION(BlueprintPure, Category = "InterfaceGuard")
	FString SummaryLine() const;

	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	void DumpToLog() const;

#if WITH_EDITOR
	/** The editor-viewport-with-no-PIE draw path. See AInterfaceGuardHUDActor::DrawPanelStatic. */
	void OnEditorViewportDraw(UCanvas* Canvas, APlayerController* PlayerController);
#endif

private:
	UPROPERTY()
	TArray<FInterfaceGuardFinding> Findings;

	int32 NumSafeCalls = 0;
	int32 NumDirectCalls = 0;
	int32 NumBlueprintPathHits = 0;
	bool bHasScannedAtLeastOnce = false;

#if WITH_EDITOR
	FDelegateHandle EditorDrawHandle;
#endif
};
