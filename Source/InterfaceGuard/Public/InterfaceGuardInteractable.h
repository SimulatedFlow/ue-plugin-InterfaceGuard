// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InterfaceGuardInteractable.generated.h"

/**
 * The watched interface. One BlueprintNativeEvent, which is the whole footgun: UnrealHeaderTool
 * generates a static Execute_OnInteract(UObject*) wrapper that calls Object->ProcessEvent(...) -
 * the only call path that respects a Blueprint override of OnInteract. A direct C++ call,
 * either through this interface's own vtable or by calling a derived class's
 * OnInteract_Implementation() directly, resolves through the native vtable instead, which a
 * Blueprint-only subclass never rewrites.
 *
 * See AInterfaceGuardDemoPedestal for the base ("red") behaviour and
 * UInterfaceGuardStatics::HasBlueprintOverride for the one fact (UFunction::Script.Num() > 0)
 * that tells the two call paths apart.
 */
UINTERFACE(BlueprintType)
class INTERFACEGUARD_API UInterfaceGuardDemoInteractable : public UInterface
{
	GENERATED_BODY()
};

class INTERFACEGUARD_API IInterfaceGuardDemoInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InterfaceGuard")
	void OnInteract();

	// Empty on purpose: this is the interface's own fallback, never meant to run. Every concrete
	// class that implements this interface provides its own override (see
	// AInterfaceGuardDemoPedestal::OnInteract_Implementation), and that native override is exactly
	// what a Blueprint-only subclass cannot replace - which is the mistake this whole plugin exists
	// to catch.
	virtual void OnInteract_Implementation() {}
};
