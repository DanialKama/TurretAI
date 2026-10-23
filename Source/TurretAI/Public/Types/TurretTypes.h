// Copyright 2026 Danial Kamali. All Rights Reserved.

#pragma once

#include "TurretTypes.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETurretAbility
{
	None			= 0x00	UMETA(Hidden),
	ExplosiveShot	= 0x01,
	Homing			= 0x02,
};
ENUM_CLASS_FLAGS(ETurretAbility);

/**
 * Used in turret class to initialize it
 */
USTRUCT(BlueprintType)
struct TURRETAI_API FTurretInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0", UIMin = "0"))
	float FireRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0", UIMin = "0"))
	float MaxPitch = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMax = "0", UIMax = "0"))
	float MinPitch = -45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0", UIMin = "0"))
	float RotationSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (Bitmask, BitmaskEnum = "/Script/TurretAI.ETurretAbility"))
	int32 TurretAbility = 0;

	void SetFlag(ETurretAbility Flag)
	{
		TurretAbility |= static_cast<int32>(Flag);
	}

	bool HasFlag(ETurretAbility Flag) const
	{
		return (TurretAbility & static_cast<int32>(Flag)) == static_cast<int32>(Flag);
	}
};
