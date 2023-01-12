// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Cannon.h"
#include "CannonV2.generated.h"

/**
 * This version of the cannon includes a base, a turret, and barrel
 */
UCLASS(meta = (DisplayName = "Cannon V2 Actor"))
class CANNONAI_API ACannonV2 : public ACannon
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> TurretMesh;

// Functions
public:
	/** Sets default values for this actor's properties */
	ACannonV2(/*const FObjectInitializer& ObjectInitializer*/);
	
protected:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	virtual void MulticastDestroyCannon_Implementation() override;

	virtual void StartSink() const override;
};
