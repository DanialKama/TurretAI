// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/Projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);
	InitialLifeSpan = 5.0f;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	RootComponent = ProjectileMesh;
	ProjectileMesh->SetLinearDamping(0.0f);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	ProjectileMesh->CanCharacterStepUpOn = ECB_No;
	ProjectileMesh->SetCollisionProfileName("BlockAllDynamic");
	ProjectileMesh->SetNotifyRigidBodyCollision(true);	// Simulation Generates Hit Events
	ProjectileMesh->SetCanEverAffectNavigation(false);

	TrailParticle = CreateDefaultSubobject<UNiagaraComponent>("TrailParticle");
	TrailParticle->SetupAttachment(ProjectileMesh);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	
	// Initialize variables
	bDoOnceHit = true;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMesh->IgnoreActorWhenMoving(GetOwner(), true);

	ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectile::ProjectileHit);

	if (HomingTarget)
	{
		ProjectileMovement->HomingTargetComponent = HomingTarget;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		ProjectileMovement->bIsHomingProjectile = true;
	}
}

void AProjectile::ProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bDoOnceHit)
	{
		bDoOnceHit = false;
		ProjectileMesh->SetNotifyRigidBodyCollision(false);
		
		// Is server
		if (GetWorld()->GetNetMode() != NM_Client)
		{
			if (ProjectileAbility & Explosive)
			{
				ApplyExplosiveHit(Hit);
			}
			else
			{
				ApplyNormalHit(Hit);
			}
		}
		
		DisableProjectile();
	}
}

void AProjectile::ApplyNormalHit(const FHitResult& HitResult) const
{
	UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), DamageInfo.BaseDamage, HitResult.ImpactNormal, HitResult,
		GetInstigatorController(), GetOwner(), nullptr);
}

void AProjectile::ApplyExplosiveHit(const FHitResult& HitResult) const
{
	const TArray<AActor*> Actors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), DamageInfo.BaseDamage, DamageInfo.MinimumDamage, HitResult.ImpactPoint,
		DamageInfo.InnerRadius, DamageInfo.OuterRadius, 1.0f, nullptr, Actors, GetOwner(), GetInstigatorController());
}

void AProjectile::DisableProjectile()
{
	FFXSystemSpawnParameters SpawnParams;
	SpawnParams.WorldContextObject = GetWorld();
	SpawnParams.SystemTemplate = HitParticle;
	SpawnParams.Location = GetActorLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitSound, GetActorLocation());

	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetVisibility(false);
		
	TrailParticle->Deactivate();
		
	SetLifeSpan(2.0f);
}
