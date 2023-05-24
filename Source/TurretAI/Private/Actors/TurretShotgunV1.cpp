// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/TurretShotgunV1.h"

void ATurretShotgunV1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const FRotator NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, TurretInfo.MinPitch, TurretInfo.MaxPitch), NewRotation.Yaw, 0.0f));
}
