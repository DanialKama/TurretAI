// Copyright 2026 Danial Kamali. All Rights Reserved.

#include "Actors/TurretShotgunV1.h"

void ATurretShotgunV1::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FRotator NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, MinPitch, MaxPitch), NewRotation.Yaw, 0.0f));
}
