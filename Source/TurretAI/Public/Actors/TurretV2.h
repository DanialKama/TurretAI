// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Turret.h"
#include "TurretV2.generated.h"

/**
 * This version of the turret includes a base, a turret, and barrel
 */
UCLASS(Blueprintable, meta = (DisplayName = "Turret AI V2"))
class TURRETAI_API ATurretV2 : public ATurret
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> TurretMesh;

// Functions
public:
	/** Sets default values for this actor's properties */
	ATurretV2();
	
protected:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	virtual void MulticastDestroyTurret_Implementation() override;

	virtual void StartSink() const override;
};
