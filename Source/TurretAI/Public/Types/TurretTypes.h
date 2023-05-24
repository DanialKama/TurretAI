// Copyright 2023 Danial Kamali. All Rights Reserved.

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = 0.0, UIMin = 0.0))
	float FireRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = 0.0, UIMin = 0.0))
	float MaxPitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMax = 0.0, UIMax = 0.0))
	float MinPitch;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = 0.0, UIMin = 0.0))
	float RotationSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = 0.0, UIMin = 0.0))
	float AccuracyOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (Bitmask, BitmaskEnum = "/Script/TurretAI.ETurretAbility"))
	int32 TurretAbility;

	// Default constructor
	FTurretInfo()
		: FireRate(1.0f), MaxPitch(45.0f), MinPitch(-45.0f), RotationSpeed(100.0f), AccuracyOffset(10.0f), TurretAbility(0)
	{}

	void SetFlag(ETurretAbility Flag)
	{
		TurretAbility |= static_cast<int32>(Flag);
	}
	
	bool HasFlag(ETurretAbility Flag) const
	{
		return (TurretAbility & static_cast<int32>(Flag)) == static_cast<int32>(Flag);
	}
};
