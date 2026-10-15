// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "InterfaceGuardDemoPedestal.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AInterfaceGuardDemoPedestal::AInterfaceGuardDemoPedestal()
{
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetMobility(EComponentMobility::Movable);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// THE MESH AND THE BASE MATERIAL ARE DELIBERATELY NOT SET HERE. Which mesh a pedestal shows is
	// a decision for the content phase (Phase 3, by Python script, once the build lock is free) -
	// not for C++. BodyMID is created lazily in BeginPlay, IF a base material happens to be set by
	// then; if not, the pedestal stays invisible, but the dispatch logic this plugin demonstrates
	// does not depend on it.
}

void AInterfaceGuardDemoPedestal::BeginPlay()
{
	Super::BeginPlay();

	if (Body)
	{
		if (UMaterialInterface* BaseMaterial = Body->GetMaterial(0))
		{
			BodyMID = Body->CreateDynamicMaterialInstance(0, BaseMaterial);
		}
	}

	// Starts red: nothing has been called yet, and red is also the correct answer for "no
	// Blueprint override reached" - the state this pedestal is in before any click.
	SetPedestalColour(false);
}

void AInterfaceGuardDemoPedestal::OnInteract_Implementation()
{
	// The native/base behaviour. Reached whenever dispatch lands HERE rather than in a Blueprint
	// graph override - which is every call on a pedestal with no override at all, and EVERY direct
	// C++ call on ANY pedestal, overridden or not, because a Blueprint-only subclass never rewrites
	// this native vtable slot. See the class comment for the two cases this covers.
	SetPedestalColour(false);
}

void AInterfaceGuardDemoPedestal::SetPedestalColour(bool bGreen)
{
	bIsGreen = bGreen;

	if (BodyMID)
	{
		BodyMID->SetVectorParameterValue(TEXT("Color"),
			bGreen ? FLinearColor(0.30f, 0.85f, 0.40f) : FLinearColor(0.90f, 0.20f, 0.20f));
	}
}
