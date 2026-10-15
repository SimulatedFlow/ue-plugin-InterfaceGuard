// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "InterfaceGuardDemoPedestal.h"
#include "InterfaceGuardLog.h"
#include "InterfaceGuardStatics.h"
#include "UObject/SoftObjectPath.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace InterfaceGuardTests
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext
		| EAutomationTestFlags::CommandletContext
		| EAutomationTestFlags::EngineFilter;
}

// -------------------------------------------------------------------------------------------
// 1. A pure native class has no Blueprint override - HasBlueprintOverride says so and explains
//    why, rather than just returning false on its own.
// -------------------------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterfaceGuardNativePedestalHasNoOverrideTest,
	"InterfaceGuard.Rules.NativePedestalHasNoOverride", InterfaceGuardTests::TestFlags)

bool FInterfaceGuardNativePedestalHasNoOverrideTest::RunTest(const FString&)
{
	FString Reason;
	const bool bHasOverride = UInterfaceGuardStatics::HasBlueprintOverride(
		AInterfaceGuardDemoPedestal::StaticClass(), TEXT("OnInteract"), Reason);

	TestFalse(TEXT("a pure native pedestal has no Blueprint override"), bHasOverride);
	TestFalse(TEXT("the reason is never left empty"), Reason.IsEmpty());

	return true;
}

// -------------------------------------------------------------------------------------------
// 2. A function name that does not exist on the class is reported cleanly, not as an override
//    and not as a crash.
// -------------------------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterfaceGuardUnknownFunctionIsReportedCleanlyTest,
	"InterfaceGuard.Rules.UnknownFunctionIsReportedCleanly", InterfaceGuardTests::TestFlags)

bool FInterfaceGuardUnknownFunctionIsReportedCleanlyTest::RunTest(const FString&)
{
	FString Reason;
	const bool bHasOverride = UInterfaceGuardStatics::HasBlueprintOverride(
		AInterfaceGuardDemoPedestal::StaticClass(), TEXT("ThisFunctionDoesNotExist"), Reason);

	TestFalse(TEXT("a function that does not exist is never reported as overridden"), bHasOverride);
	TestFalse(TEXT("the reason explains why"), Reason.IsEmpty());

	// And a null class is the same kind of clean "no" - not a crash.
	FString NullReason;
	const bool bNullHasOverride = UInterfaceGuardStatics::HasBlueprintOverride(nullptr, TEXT("OnInteract"), NullReason);
	TestFalse(TEXT("a null class reports false"), bNullHasOverride);
	TestFalse(TEXT("a null class still gets an explanation"), NullReason.IsEmpty());

	return true;
}

// -------------------------------------------------------------------------------------------
// 3. The direct-call antipattern always lands on the native implementation, on a plain native
//    pedestal, with no Blueprint involved at all - the mechanism, not just the detector.
// -------------------------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterfaceGuardDirectCallHitsNativeImplementationTest,
	"InterfaceGuard.Demo.DirectCallHitsNativeImplementation", InterfaceGuardTests::TestFlags)

bool FInterfaceGuardDirectCallHitsNativeImplementationTest::RunTest(const FString&)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld=*/false);
	if (World == nullptr)
	{
		AddWarning(TEXT("Could not spin up a transient world for this test - skipping."));
		return true;
	}

	AInterfaceGuardDemoPedestal* Pedestal = World->SpawnActor<AInterfaceGuardDemoPedestal>();
	bool bOk = true;

	if (Pedestal == nullptr)
	{
		AddError(TEXT("Could not spawn AInterfaceGuardDemoPedestal."));
		bOk = false;
	}
	else
	{
		Pedestal->SetPedestalColour(true);
		TestTrue(TEXT("the pedestal starts green for this check"), Pedestal->IsShowingGreen());

		const bool bCalled = UInterfaceGuardStatics::CallInterfaceFunctionDirectly_WRONG_WAY(Pedestal);
		TestTrue(TEXT("the direct call was performed"), bCalled);
		TestFalse(TEXT("the direct call turned the pedestal red - the native implementation ran"),
			Pedestal->IsShowingGreen());
	}

	World->DestroyWorld(/*bInformEngineOfWorld=*/false);
	return bOk;
}

// -------------------------------------------------------------------------------------------
// 4. The Blueprint override itself is content, authored in Phase 3 of the build pipeline (its
//    own map and widgets), not part of this C++ module - so this test is written to pass BOTH
//    before and after that content exists: it looks for BP_InterfaceGuardPedestal_Overridden
//    and skips with a warning, rather than failing, when the asset has not been created yet in
//    this session.
// -------------------------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInterfaceGuardBlueprintOverrideIsDetectedWhenPresentTest,
	"InterfaceGuard.Demo.BlueprintOverrideIsDetectedWhenPresent", InterfaceGuardTests::TestFlags)

bool FInterfaceGuardBlueprintOverrideIsDetectedWhenPresentTest::RunTest(const FString&)
{
	const FSoftObjectPath BlueprintPath(
		TEXT("/InterfaceGuard/Blueprints/BP_InterfaceGuardPedestal_Overridden.BP_InterfaceGuardPedestal_Overridden"));
	UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintPath.TryLoad());

	if (Blueprint == nullptr || Blueprint->GeneratedClass == nullptr)
	{
		AddWarning(TEXT("BP_InterfaceGuardPedestal_Overridden is not loadable in this test context yet "
			"(the demo content is authored in the pipeline's content phase) - skipping, not failing."));
		return true;
	}

	FString Reason;
	const bool bHasOverride = UInterfaceGuardStatics::HasBlueprintOverride(
		Blueprint->GeneratedClass, TEXT("OnInteract"), Reason);

	TestTrue(TEXT("the Blueprint override is detected"), bHasOverride);
	TestFalse(TEXT("the reason is never left empty"), Reason.IsEmpty());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
