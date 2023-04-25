// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "DestroyedStructure.h"

#include "Components/StaticMeshComponent.h"

ADestroyedStructure::ADestroyedStructure()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);
	InitialLifeSpan = 6.0f;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMesh;
	StaticMesh->bApplyImpulseOnDamage = false;
	StaticMesh->SetCollisionProfileName("Destructible");
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetNotifyRigidBodyCollision(true);
	StaticMesh->SetCanEverAffectNavigation(false);

	// Initialize variables
	bDoOnceHit = true;
}

void ADestroyedStructure::Initialize(UStaticMesh* InMesh, TArray<UMaterialInterface*> Materials, const float InLinearDamping, const float InAngularDamping) const
{
	StaticMesh->SetStaticMesh(InMesh);

	for (int32 i = 0; i < Materials.Num(); ++i)
	{
		StaticMesh->SetMaterial(i, Materials[i]);
	}

	StaticMesh->SetLinearDamping(InLinearDamping);
	StaticMesh->SetAngularDamping(InAngularDamping);

	StaticMesh->SetSimulatePhysics(true);

	StaticMesh->OnComponentHit.AddDynamic(this, &ADestroyedStructure::MeshHit);
}

void ADestroyedStructure::MeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bDoOnceHit)
	{
		bDoOnceHit = false;
		StaticMesh->SetNotifyRigidBodyCollision(false);

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ADestroyedStructure::StartSink, GetLifeSpan() - 2.0f);
	}
}

void ADestroyedStructure::StartSink() const
{
	StaticMesh->SetLinearDamping(1.0f);
	StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
