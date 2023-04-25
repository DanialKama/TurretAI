// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/TurretV2.h"

ATurretV2::ATurretV2()
{
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
	TurretMesh->bReplicatePhysicsToAutonomousProxy = false;
	TurretMesh->SetGenerateOverlapEvents(false);
	TurretMesh->CanCharacterStepUpOn = ECB_No;
	TurretMesh->SetCanEverAffectNavigation(false);
	
	BarrelMesh->SetupAttachment(TurretMesh);
}

void ATurretV2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = CalculateRotation(TurretMesh, DeltaTime);
	TurretMesh->SetRelativeRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, TurretInfo.MinPitch, TurretInfo.MaxPitch), 0.0f, 0.0f));
}

void ATurretV2::MulticastDestroyTurret_Implementation()
{
	Super::MulticastDestroyTurret_Implementation();
	
	TurretMesh->SetCollisionProfileName("Destructible");
	TurretMesh->SetSimulatePhysics(true);
}

void ATurretV2::StartSink() const
{
	Super::StartSink();
	
	TurretMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
