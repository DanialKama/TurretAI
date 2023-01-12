// All Rights Reserved.

#pragma once

// #include "Engine/DataTable.h"
#include "CannonInfo.generated.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECannonAbility
{
	None			= 0x00	UMETA(Hidden),
	ExplosiveShot	= 0x01,
	Homing			= 0x02,
	Shotgun			= 0x04
};

ENUM_CLASS_FLAGS(ECannonAbility);

/**
 * Used in cannon class to initialize it
 */
USTRUCT(BlueprintType)
struct CANNONAI_API FCannonInfo /*: public FTableRowBase*/
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float FireRate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxPitch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MinPitch;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RotationSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AccuracyOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "/Script/CannonAI.ECannonAbility"))
	int32 CannonAbility;

	FCannonInfo()
		: FireRate(1.0f), MaxPitch(45.0f), MinPitch(-45.0f), RotationSpeed(100.0f), AccuracyOffset(10.0f), CannonAbility(0)
	{}

	FCannonInfo(const float InFireRate, const float InMaxPitch, const float InMinPitch, const float InRotationSpeed, const float InAccuracyOffset, const int32 InCannonAbility)
		: FireRate(InFireRate), MaxPitch(InMaxPitch), MinPitch(InMinPitch), RotationSpeed(InRotationSpeed), AccuracyOffset(InAccuracyOffset), CannonAbility(InCannonAbility)
	{}
};
