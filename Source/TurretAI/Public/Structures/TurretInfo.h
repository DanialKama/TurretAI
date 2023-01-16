// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

// #include "Engine/DataTable.h"
#include "TurretInfo.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETurretAbility
{
	None			= 0x00	UMETA(Hidden),
	ExplosiveShot	= 0x01,
	Homing			= 0x02,
	Shotgun			= 0x04
};

ENUM_CLASS_FLAGS(ETurretAbility);

/**
 * Used in turret class to initialize it
 */
USTRUCT(BlueprintType)
struct TURRETAI_API FTurretInfo /*: public FTableRowBase*/
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float FireRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxPitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMax = "0.0", UIMax = "0.0"))
	float MinPitch;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RotationSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float AccuracyOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Turret", meta = (Bitmask, BitmaskEnum = "/Script/TurretAI.ETurretAbility"))
	int32 TurretAbility;

	FTurretInfo()
		: FireRate(1.0f), MaxPitch(45.0f), MinPitch(-45.0f), RotationSpeed(100.0f), AccuracyOffset(10.0f), TurretAbility(0)
	{}

	FTurretInfo(const float InFireRate, const float InMaxPitch, const float InMinPitch, const float InRotationSpeed, const float InAccuracyOffset, const int32 InTurretAbility)
		: FireRate(InFireRate), MaxPitch(InMaxPitch), MinPitch(InMinPitch), RotationSpeed(InRotationSpeed), AccuracyOffset(InAccuracyOffset), TurretAbility(InTurretAbility)
	{}
};
