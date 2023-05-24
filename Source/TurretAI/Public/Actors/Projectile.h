// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraSystem;

enum EProjectileAbility
{
	None		= 0x00,
	Explosive	= 0x01
};

/**
 * Base class for projectiles
 */
UCLASS(meta = (DisplayName = "Projectile"))
class TURRETAI_API AProjectile : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UNiagaraComponent> TrailParticle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

// Functions
public:
	/** Sets default values for this actor's properties */
	AProjectile();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

private:
	void LoadAssets();
	
	UFUNCTION()
	void ProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyNormalHit(const FHitResult& HitResult) const;
	void ApplyExplosiveHit(const FHitResult& HitResult) const;

	/** Disabling the projectile after hit and destroying it with a delay so trail particles have time to disappear */
	void DisableProjectile();

// Variables
public:
	TWeakObjectPtr<USceneComponent> HomingTarget;

	uint8 ProjectileAbility = 0;

private:
	/** For non-explosive projectiles, only Base Damage is required */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	FRadialDamageParams DamageInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UNiagaraSystem> HitParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<USoundBase> HitSound;
	
	UPROPERTY()
	UNiagaraSystem* HitParticleLoaded;

	UPROPERTY()
	USoundBase* HitSoundLoaded;
	
	uint8 bDoOnceHit : 1;
};
