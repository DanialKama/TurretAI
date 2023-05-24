// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Turret.h"
#include "TurretShotgunV1.generated.h"

/**
 * This version of the turret includes a base and a barrel
 */
UCLASS(Blueprintable, meta = (DisplayName = "Shotgun Turret AI V1"))
class TURRETAI_API ATurretShotgunV1 : public ATurret
{
	GENERATED_BODY()

// Functions
public:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
};
