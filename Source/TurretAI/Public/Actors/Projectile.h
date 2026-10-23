// Copyright 2026 Danial Kamali. All Rights Reserved.

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
UCLASS(meta = (DisplayName = "Default"))
class TURRETAI_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> TrailParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

public:
	AProjectile();

	void SetFlag(const EProjectileAbility Flag)
	{
		ProjectileAbility |= static_cast<int32>(Flag);
	}

	bool HasFlag(const EProjectileAbility Flag) const
	{
		return (ProjectileAbility & static_cast<int32>(Flag)) == static_cast<int32>(Flag);
	}

protected:
	virtual void BeginPlay() override;

private:
	void LoadAssets();

	UFUNCTION()
	void ProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyNormalHit(const FHitResult& HitResult) const;
	void ApplyExplosiveHit(const FHitResult& HitResult) const;

	/** Disabling the projectile after hit and destroying it with a delay so trail particles have time to disappear */
	void DisableProjectile();

public:
	TWeakObjectPtr<USceneComponent> HomingTarget;

	uint8 ProjectileAbility = 0;

private:
	/** For non-explosive projectiles, only Base Damage is required */
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FRadialDamageParams DamageInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<UNiagaraSystem> HitParticle;

	UPROPERTY(Transient)
	UNiagaraSystem* HitParticleLoaded = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<USoundBase> HitSound;

	UPROPERTY(Transient)
	USoundBase* HitSoundLoaded = nullptr;

	uint8 bDoOnceHit : 1 = true;
};
