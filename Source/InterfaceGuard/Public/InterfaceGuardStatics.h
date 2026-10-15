// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InterfaceGuardStatics.generated.h"

/**
 * The whole check, plus the one correct call path and the one wrong call path, as three static
 * functions.
 *
 * HasBlueprintOverride is a pure reflection question - no world, no network, no Asset Registry -
 * so it is covered by automation tests and usable in a cooked -game build exactly as it is in the
 * editor. CallInterfaceFunctionSafely and CallInterfaceFunctionDirectly_WRONG_WAY exist so the
 * demo's two buttons, and this plugin's own tests, exercise the real mechanism rather than a
 * description of it.
 */
UCLASS()
class INTERFACEGUARD_API UInterfaceGuardStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * The one fact that tells a Blueprint override apart from a native-only function: does
	 * TargetClass's own UFunction for FunctionName carry compiled Blueprint script bytecode?
	 *
	 * UFunction::Script is empty for a function that is purely native (declared BlueprintNativeEvent
	 * and never touched in any Blueprint graph) and non-empty the moment a Blueprint overrides the
	 * event - that bytecode is exactly what ProcessEvent (and therefore Execute_) runs, and exactly
	 * what a direct C++ call to the _Implementation function never reaches, because a Blueprint-only
	 * subclass has no native vtable slot to put a replacement in.
	 *
	 * Returns false (with an explanatory OutReason) when TargetClass is null or has no function by
	 * that name - that is not an error, it means there is nothing here a direct call could get wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	static bool HasBlueprintOverride(UClass* TargetClass, FName FunctionName, FString& OutReason);

	/**
	 * The correct call path: Target->ProcessEvent(Target->FindFunction(FunctionName), nullptr).
	 *
	 * This is a generic stand-in for the UHT-generated Execute_ wrapper, deliberately limited to a
	 * parameterless void function - the real wrappers are the right tool once an interface function
	 * has parameters or a return value (see "What is explicitly NOT built" in the documentation).
	 * What matters for the demo and the tests is that this goes through ProcessEvent exactly as
	 * Execute_ does, which is what makes it reach a Blueprint override at all.
	 *
	 * Reports the call to the Target's UInterfaceGuardSubsystem, using HasBlueprintOverride on the
	 * Target's own class to decide whether this call actually landed on a Blueprint override - so
	 * the HUD's "Blueprint hits" counter is a fact about what happened, not a guess.
	 */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	static bool CallInterfaceFunctionSafely(UObject* Target, FName FunctionName);

	/**
	 * The antipattern, deliberately not hidden: casts Target to AInterfaceGuardDemoPedestal and
	 * calls OnInteract_Implementation() directly, bypassing ProcessEvent entirely.
	 *
	 * This is a direct C++ virtual call. It always resolves to
	 * AInterfaceGuardDemoPedestal::OnInteract_Implementation() - the lowest NATIVE override in the
	 * vtable - no matter which pedestal Target actually is, because a purely Blueprint subclass
	 * such as BP_InterfaceGuardPedestal_Overridden never adds a new native vtable slot. That is the
	 * whole footgun this plugin exists to catch, demonstrated rather than just described.
	 *
	 * Reports the call to the Target's UInterfaceGuardSubsystem as a direct call.
	 */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard")
	static bool CallInterfaceFunctionDirectly_WRONG_WAY(UObject* Target);
};
