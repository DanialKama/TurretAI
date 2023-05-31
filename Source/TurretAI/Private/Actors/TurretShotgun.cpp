// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/TurretShotgun.h"

#include "Components/StaticMeshComponent.h"

void ATurretShotgun::HandleFireTurret()
{
	// Calculating direction for projectiles based on the Accuracy Offset
	TArray<FRotator> Rotations;
	Rotations.Reserve(3);
	
	const FRotator SocketRotation = BarrelMesh->GetSocketRotation("ProjectileSocket");

	uint8 i = 0;
	while (i < NumOfShots)
	{
		FRotator NewRotation;
			
		NewRotation.Pitch	= SocketRotation.Pitch	+ FMath::RandRange(ShotgunSpread * -1.0f, ShotgunSpread);
		NewRotation.Yaw		= SocketRotation.Yaw	+ FMath::RandRange(ShotgunSpread * -1.0f, ShotgunSpread);
		NewRotation.Roll	= SocketRotation.Roll	+ FMath::RandRange(ShotgunSpread * -1.0f, ShotgunSpread);

		Rotations.Add(NewRotation);
		++i;
	}
	
	MulticastFireShotgunTurret(Rotations);
}

void ATurretShotgun::MulticastFireShotgunTurret_Implementation(const TArray<FRotator>& Rotations)
{
	if (CurrentTarget == nullptr)
	{
		return;
	}
	
	FTransform NewTransform = BarrelMesh->GetSocketTransform("ProjectileSocket");
	for (FRotator NewRotation : Rotations)
	{
		NewTransform.SetRotation(NewRotation.Quaternion());
		
		SpawnProjectile(NewTransform);
	}

	SpawnFireFX();
}
