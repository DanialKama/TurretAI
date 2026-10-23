// Copyright 2026 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurretShotgun.h"
#include "TurretShotgunV2.generated.h"

/**
 * This version of the turret includes a base, a turret, and barrel
 */
UCLASS(Abstract, Blueprintable, meta = (DisplayName = "Shotgun Turret AI V2"))
class TURRETAI_API ATurretShotgunV2 : public ATurretShotgun
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

public:
	ATurretShotgunV2();

	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
};
