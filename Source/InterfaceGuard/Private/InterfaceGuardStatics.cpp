// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardStatics.h"

#include "InterfaceGuardDemoPedestal.h"
#include "InterfaceGuardLog.h"
#include "InterfaceGuardSubsystem.h"

#include "UObject/Class.h"
#include "UObject/UnrealType.h"

bool UInterfaceGuardStatics::HasBlueprintOverride(UClass* TargetClass, FName FunctionName, FString& OutReason)
{
	OutReason.Reset();

	if (TargetClass == nullptr)
	{
		OutReason = TEXT("no class given - there is nothing to check");
		return false;
	}

	UFunction* Function = TargetClass->FindFunctionByName(FunctionName);
	if (Function == nullptr)
	{
		OutReason = FString::Printf(
			TEXT("%s has no function named %s - there is nothing a direct call could get wrong"),
			*TargetClass->GetName(), *FunctionName.ToString());
		return false;
	}

	// THE ONE FACT THAT DECIDES EVERYTHING. A non-empty Script array means a Blueprint graph
	// compiled real bytecode onto this UFunction - which only ProcessEvent (and therefore only the
	// UHT-generated Execute_ wrapper) will ever execute. An empty Script array means the function is
	// purely native: calling Execute_ and calling the _Implementation function directly end up in
	// exactly the same place, so a direct call here is harmless, not a bug.
	const bool bHasBlueprintOverride = Function->Script.Num() > 0;

	OutReason = bHasBlueprintOverride
		? FString::Printf(
			TEXT("%s::%s has a Blueprint override - a direct C++ call would skip it, call via Execute_ instead"),
			*TargetClass->GetName(), *FunctionName.ToString())
		: FString::Printf(
			TEXT("%s::%s has no Blueprint override - it runs the native implementation either way"),
			*TargetClass->GetName(), *FunctionName.ToString());

	return bHasBlueprintOverride;
}

bool UInterfaceGuardStatics::CallInterfaceFunctionSafely(UObject* Target, FName FunctionName)
{
	if (Target == nullptr)
	{
		return false;
	}

	UFunction* Function = Target->FindFunction(FunctionName);
	if (Function == nullptr)
	{
		UE_LOG(LogInterfaceGuard, Warning,
			TEXT("CallInterfaceFunctionSafely: %s has no function named %s."),
			*Target->GetClass()->GetName(), *FunctionName.ToString());
		return false;
	}

	FString Reason;
	const bool bWillHitBlueprintOverride = HasBlueprintOverride(Target->GetClass(), FunctionName, Reason);

	// THE CORRECT CALL. Same mechanism the UHT-generated Execute_ wrapper uses: ProcessEvent reads
	// the UFunction's own Script bytecode when present, which is exactly what makes this reach a
	// Blueprint override that a direct call would skip.
	Target->ProcessEvent(Function, nullptr);

	if (UInterfaceGuardSubsystem* Subsystem = UInterfaceGuardSubsystem::Get(Target))
	{
		Subsystem->ReportSafeCall(bWillHitBlueprintOverride);
	}

	return true;
}

bool UInterfaceGuardStatics::CallInterfaceFunctionDirectly_WRONG_WAY(UObject* Target)
{
	AInterfaceGuardDemoPedestal* Pedestal = Cast<AInterfaceGuardDemoPedestal>(Target);
	if (Pedestal == nullptr)
	{
		UE_LOG(LogInterfaceGuard, Warning,
			TEXT("CallInterfaceFunctionDirectly_WRONG_WAY: Target is not an AInterfaceGuardDemoPedestal."));
		return false;
	}

	// THE ANTIPATTERN. A plain C++ virtual call, resolved at compile time against the native
	// vtable. Whatever Pedestal's actual runtime class is - including a Blueprint child that
	// overrides OnInteract in its event graph - this always runs
	// AInterfaceGuardDemoPedestal::OnInteract_Implementation(), because that Blueprint child never
	// added a new native vtable slot for it. No warning, no error: it compiles exactly like a
	// correct call.
	Pedestal->OnInteract_Implementation();

	if (UInterfaceGuardSubsystem* Subsystem = UInterfaceGuardSubsystem::Get(Pedestal))
	{
		Subsystem->ReportDirectCall();
	}

	return true;
}
