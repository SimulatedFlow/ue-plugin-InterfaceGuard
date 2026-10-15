// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InterfaceGuardInteractable.h"
#include "InterfaceGuardDemoPedestal.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * The pedestal. A plain C++ actor that implements IInterfaceGuardDemoInteractable with the base
 * ("red") behaviour - the behaviour that runs whenever dispatch lands here instead of in a
 * Blueprint graph.
 *
 * That happens in exactly two situations, and the demo map is built to show the difference
 * between them on two pedestals:
 *
 *   1. There is no Blueprint override at all (Pedestal A, this class used directly). Calling
 *      Execute_OnInteract correctly falls back to this implementation - InterfaceGuard does not
 *      paint every direct call red, only the ones that skip a real override.
 *   2. There IS a Blueprint override (Pedestal B, BP_InterfaceGuardPedestal_Overridden, a
 *      Blueprint child of this class, authored in the content phase) but the caller used the
 *      direct-call antipattern instead of Execute_. A Blueprint-only subclass cannot override this
 *      native virtual function - it can only replace OnInteract's UFunction script, which only
 *      ProcessEvent (and therefore only Execute_) ever reads. So OnInteract_Implementation() here
 *      is what a direct call reaches even on the overridden pedestal.
 *
 * Mesh and base material are content-phase work, same as every demo actor in this pipeline - not
 * C++'s job. With neither set, SetPedestalColour degrades to updating bIsGreen only, which keeps
 * the automation tests meaningful with no content loaded at all.
 */
UCLASS()
class INTERFACEGUARD_API AInterfaceGuardDemoPedestal : public AActor, public IInterfaceGuardDemoInteractable
{
	GENERATED_BODY()

public:
	AInterfaceGuardDemoPedestal();

	virtual void BeginPlay() override;

	//~ Begin IInterfaceGuardDemoInteractable interface
	virtual void OnInteract_Implementation() override;
	//~ End IInterfaceGuardDemoInteractable interface

	/**
	 * Sets the pedestal's colour and remembers it.
	 *
	 * Exposed as BlueprintCallable so a Blueprint override of OnInteract (the content-phase
	 * BP_InterfaceGuardPedestal_Overridden) can turn the pedestal green without re-implementing
	 * the material logic in the graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "InterfaceGuard|Demo")
	void SetPedestalColour(bool bGreen);

	UFUNCTION(BlueprintPure, Category = "InterfaceGuard|Demo")
	bool IsShowingGreen() const { return bIsGreen; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "InterfaceGuard|Demo")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;

	bool bIsGreen = false;
};
