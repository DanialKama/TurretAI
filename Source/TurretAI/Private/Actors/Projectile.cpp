// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/Projectile.h"

#include "Engine/AssetManager.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetCanBeDamaged(false);
	InitialLifeSpan = 5.0f;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;
	ProjectileMesh->SetLinearDamping(0.0f);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	ProjectileMesh->CanCharacterStepUpOn = ECB_No;
	ProjectileMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	ProjectileMesh->SetNotifyRigidBodyCollision(true);
	ProjectileMesh->SetCanEverAffectNavigation(false);

	TrailParticle = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail Particle"));
	TrailParticle->SetupAttachment(ProjectileMesh);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	
	// Initialize variables
	bDoOnceHit = true;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	LoadAssets();

	ProjectileMesh->IgnoreActorWhenMoving(GetOwner(), true);

	ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectile::ProjectileHit);

	if (HomingTarget.IsValid())
	{
		ProjectileMovement->HomingTargetComponent = HomingTarget;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		ProjectileMovement->bIsHomingProjectile = true;
	}
}

void AProjectile::LoadAssets()
{
	TArray<FSoftObjectPath> Paths;

	if (HitParticle.ToSoftObjectPath().IsValid())
	{
		HitParticleLoaded = HitParticle.Get();
		if (HitParticleLoaded == nullptr)
		{
			Paths.Add(HitParticle.ToSoftObjectPath());
		}
	}

	if (HitSound.ToSoftObjectPath().IsValid())
	{
		HitSoundLoaded = HitSound.Get();
		if (HitSoundLoaded == nullptr)
		{
			Paths.Add(HitSound.ToSoftObjectPath());
		}
	}

	if (Paths.IsEmpty())
	{
		return;
	}
	
	UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate::CreateWeakLambda(this, [this]
	{
		if (HitParticleLoaded == nullptr)
		{
			HitParticleLoaded = Cast<UNiagaraSystem>(HitParticle.Get());
		}
	
		if (HitSoundLoaded == nullptr)
		{
			HitSoundLoaded = Cast<USoundBase>(HitSound.Get());
		}
	}));
}

void AProjectile::ProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bDoOnceHit == false)
	{
		return;
	}
	
	bDoOnceHit = false;
	ProjectileMesh->SetNotifyRigidBodyCollision(false);

	DisableProjectile();
	
	// Is server?
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}
	
	if (HasFlag(EProjectileAbility::Explosive))
	{
		ApplyExplosiveHit(Hit);
	}
	else
	{
		ApplyNormalHit(Hit);
	}
}

void AProjectile::ApplyNormalHit(const FHitResult& HitResult) const
{
	UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), DamageInfo.BaseDamage, HitResult.ImpactNormal, HitResult,
		GetInstigatorController(), GetOwner(), nullptr);
}

void AProjectile::ApplyExplosiveHit(const FHitResult& HitResult) const
{
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), DamageInfo.BaseDamage, DamageInfo.MinimumDamage, HitResult.ImpactPoint,
		DamageInfo.InnerRadius, DamageInfo.OuterRadius, 1.0f, nullptr, TArray<AActor*>(), GetOwner(), GetInstigatorController());
}

void AProjectile::DisableProjectile()
{
	FFXSystemSpawnParameters SpawnParams;
	SpawnParams.WorldContextObject = GetWorld();
	SpawnParams.SystemTemplate = HitParticleLoaded;
	SpawnParams.Location = ProjectileMesh->GetComponentLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(SpawnParams.WorldContextObject, HitSoundLoaded, GetActorLocation());

	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetVisibility(false);
		
	TrailParticle->Deactivate();
		
	SetLifeSpan(2.0f);
}
