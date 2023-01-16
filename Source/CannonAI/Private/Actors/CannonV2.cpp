// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/CannonV2.h"

ACannonV2::ACannonV2()
{
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>("TurretMesh");
	TurretMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
	TurretMesh->bReplicatePhysicsToAutonomousProxy = false;
	TurretMesh->SetGenerateOverlapEvents(false);
	TurretMesh->CanCharacterStepUpOn = ECB_No;
	TurretMesh->SetCanEverAffectNavigation(false);
	
	BarrelMesh->SetupAttachment(TurretMesh);
}

void ACannonV2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = CalculateRotation(TurretMesh, DeltaTime);
	TurretMesh->SetRelativeRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, CannonInfo.MinPitch, CannonInfo.MaxPitch), 0.0f, 0.0f));
}

void ACannonV2::MulticastDestroyCannon_Implementation()
{
	Super::MulticastDestroyCannon_Implementation();
	
	TurretMesh->SetCollisionProfileName("Destructible");
	TurretMesh->SetSimulatePhysics(true);
}

void ACannonV2::StartSink() const
{
	Super::StartSink();
	
	TurretMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
