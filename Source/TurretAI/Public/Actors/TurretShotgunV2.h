// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Turret.h"
#include "TurretShotgunV2.generated.h"

/**
 * This version of the turret includes a base, a turret, and barrel
 */
UCLASS(Blueprintable, meta = (DisplayName = "Shotgun Turret AI V2"))
class TURRETAI_API ATurretShotgunV2 : public ATurret
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> TurretMesh;

// Functions
public:
	/** Sets default values for this actor's properties */
	ATurretShotgunV2();
	
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
};
