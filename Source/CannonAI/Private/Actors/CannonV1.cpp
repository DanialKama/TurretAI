// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/CannonV1.h"

void ACannonV1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const FRotator NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, CannonInfo.MinPitch, CannonInfo.MaxPitch), NewRotation.Yaw, 0.0f));
}
