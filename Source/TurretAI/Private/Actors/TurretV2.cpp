// Copyright 2026 Danial Kamali. All Rights Reserved.

#include "Actors/TurretV2.h"

#include "DestroyedStructure.h"
#include "Engine/World.h"

ATurretV2::ATurretV2()
{
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret Mesh"));
	TurretMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
	TurretMesh->SetGenerateOverlapEvents(false);
	TurretMesh->CanCharacterStepUpOn = ECB_No;
	TurretMesh->SetCanEverAffectNavigation(false);

	BarrelMesh->SetupAttachment(TurretMesh);
}

void ATurretV2::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = CalculateRotation(TurretMesh, DeltaTime);
	TurretMesh->SetRelativeRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, MinPitch, MaxPitch), 0.0f, 0.0f));
}

void ATurretV2::Destroyed()
{
	if (UWorld* MyWorld = GetWorld();
		MyWorld->HasBegunPlay())
	{
		// Spawn the cannon turret
		ADestroyedStructure* NewStructure = Cast<ADestroyedStructure>(MyWorld->SpawnActor(ADestroyedStructure::StaticClass(), &TurretMesh->GetComponentTransform()));
		NewStructure->Initialize(TurretMesh->GetStaticMesh(), TurretMesh->GetMaterials(), TurretMesh->GetLinearDamping(), TurretMesh->GetAngularDamping());
	}

	Super::Destroyed();
}
