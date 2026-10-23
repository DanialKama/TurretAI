// Copyright 2026 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurretShotgun.h"
#include "TurretShotgunV1.generated.h"

/**
 * This version of the turret includes a base and a barrel
 */
UCLASS(Abstract, Blueprintable, meta = (DisplayName = "Shotgun Turret AI V1"))
class TURRETAI_API ATurretShotgunV1 : public ATurretShotgun
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
};
