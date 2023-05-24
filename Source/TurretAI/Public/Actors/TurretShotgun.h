// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Turret.h"
#include "TurretShotgun.generated.h"

/**
 * Shotgun turret AI base class
 */
UCLASS(Abstract, NotBlueprintable, meta = (DisplayName = "Shotgun Turret AI"))
class TURRETAI_API ATurretShotgun : public ATurret
{
	GENERATED_BODY()

// Functions
protected:
	virtual void HandleFireTurret() override;

private:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireShotgunTurret(const TArray<FRotator>& Rotations);
	void MulticastFireShotgunTurret_Implementation(const TArray<FRotator>& Rotations);

// Variables
private:
	/** Used to store selling info after initializing the cannon */
	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (AllowPrivateAccess = true))
	float AccuracyOffset = 5.0f;
};
