// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InterfaceGuardTypes.generated.h"

/**
 * One class that has a Blueprint override of a watched interface function - exactly the class a
 * direct C++ call would silently skip, because a Blueprint-only subclass never rewrites the
 * native vtable slot a direct call resolves through. Only looks-like-a-bug classes are ever
 * added here: a class with no override is not a finding, it is the normal case.
 */
USTRUCT(BlueprintType)
struct INTERFACEGUARD_API FInterfaceGuardFinding
{
	GENERATED_BODY()

	/** The class that has the override - native base classes never appear here. */
	UPROPERTY(BlueprintReadOnly, Category = "InterfaceGuard")
	FName ClassName;

	UPROPERTY(BlueprintReadOnly, Category = "InterfaceGuard")
	FName FunctionName;

	/** Human-readable reason, always non-empty - e.g. "has a Blueprint override - a direct C++
	    call would skip it, call via Execute_ instead". */
	UPROPERTY(BlueprintReadOnly, Category = "InterfaceGuard")
	FString Reason;
};
