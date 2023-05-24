// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "DestroyedStructure.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADestroyedStructure::ADestroyedStructure()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);
	InitialLifeSpan = 6.0f;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMesh;
	StaticMesh->bApplyImpulseOnDamage = false;
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetNotifyRigidBodyCollision(true);
	StaticMesh->SetCanEverAffectNavigation(false);

	// NOTE: You can create a new collision profile and use it
	StaticMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	StaticMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

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
	if (bDoOnceHit == false)
	{
		return;
	}
	
	bDoOnceHit = false;
	StaticMesh->SetNotifyRigidBodyCollision(false);

	const float Delay = GetLifeSpan() - 2.0f;
	if (Delay > 0.0f)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ADestroyedStructure::StartSink, Delay);
	}
	else
	{
		StartSink();
	}
}

void ADestroyedStructure::StartSink() const
{
	StaticMesh->SetLinearDamping(1.0f);
	StaticMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
