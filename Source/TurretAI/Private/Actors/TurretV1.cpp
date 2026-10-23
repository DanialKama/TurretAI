// Copyright 2026 Danial Kamali. All Rights Reserved.

#include "Actors/TurretV1.h"

void ATurretV1::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FRotator NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, MinPitch, MaxPitch), NewRotation.Yaw, 0.0f));
}
