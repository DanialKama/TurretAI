// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Turret.h"
#include "TurretV1.generated.h"

/**
 * This version of the turret includes a base and a barrel
 */
UCLASS(Blueprintable, meta = (DisplayName = "Turret AI V1"))
class TURRETAI_API ATurretV1 : public ATurret
{
	GENERATED_BODY()

// Functions
protected:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
};
