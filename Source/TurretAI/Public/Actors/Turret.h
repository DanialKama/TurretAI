// Copyright 2026 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class UNiagaraSystem;

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETurretAbility
{
	None			= 0x00	UMETA(Hidden),
	ExplosiveShot	= 0x01,
	Homing			= 0x02,
};
ENUM_CLASS_FLAGS(ETurretAbility);

/**
 * Turret AI base class
 */
UCLASS(Abstract, NotBlueprintable, meta = (DisplayName = "Turret AI"))
class TURRETAI_API ATurret : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class USphereComponent> Detector;

public:
	ATurret();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

private:
	UPROPERTY(EditAnywhere, Category = "Default", meta = (ClampMin = "0", UIMin = "0"))
	float CurrentHealth = 100.0f;

protected:
	virtual void HandleFireTurret();

	void SpawnProjectile(const FTransform& Transform);

	void SpawnFireFX() const;

	/** Calculating the target rotation so the turret will smoothly rotate toward the target */
	FRotator CalculateRotation(const UStaticMeshComponent* CompToRotate, const float DeltaTime) const;

private:
	UFUNCTION()
	void DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** On losing the current target, switch to another target if there is any. */
	void FindNewTarget();

	/** @note Should not be called directly, use FindNewTarget() */
	void FindNewTargetImpl();

	/** Trying to fire the turret based on the current state of the target (enemy). */
	void StartFireTurret();

	/** Handling firing the turret */
	void FireTurret();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireTurret();
	void MulticastFireTurret_Implementation();

	/** Finding a new random rotation for the turret to use when there is no enemy */
	void FindRandomRotation();

	/** A simple test to make sure that the turret can see the target and target is not behind any cover */
	bool CanSeeTarget(const AActor* Target) const;

	/**
	* Checking the target state and see that can projectile hit the target
	* @param	Target	Target actor that we try to hit
	* @return	True if the projectile can hit the target
	*/
	virtual bool CanHitTarget(AActor* Target) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (ClampMin = "0", UIMin = "0"))
	float FireRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (ClampMin = "0", UIMin = "0"))
	float MaxPitch = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (ClampMax = "0", UIMax = "0"))
	float MinPitch = -45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (ClampMin = "0", UIMin = "0"))
	float RotationSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Default", meta = (Bitmask, BitmaskEnum = "/Script/TurretAI.ETurretAbility"))
	int32 TurretAbility = 0;

private:
	void SetFlag(ETurretAbility Flag)
	{
		TurretAbility |= static_cast<int32>(Flag);
	}

	bool HasFlag(ETurretAbility Flag) const
	{
		return (TurretAbility & static_cast<int32>(Flag)) == static_cast<int32>(Flag);
	}

protected:
	/** The current enemy that the turret try to shoot at it */
	UPROPERTY(Replicated)
	AActor* CurrentTarget;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftClassPtr<class AProjectile> Projectile;

	UPROPERTY(Transient)
	UClass* ProjectileLoaded = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<UNiagaraSystem> FireParticle;

	UPROPERTY(Transient)
	UNiagaraSystem* FireParticleLoaded = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<USoundBase> FireSound;

	UPROPERTY(Transient)
	USoundBase* FireSoundLoaded = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<UNiagaraSystem> DestroyParticle;

	UPROPERTY(Transient)
	UNiagaraSystem* DestroyParticleLoaded = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	TSoftObjectPtr<USoundBase> DestroySound;

	UPROPERTY(Transient)
	USoundBase* DestroySoundLoaded = nullptr;

	/** Target rotation that the turret will try to look at when there is no enemy */
	UPROPERTY(Replicated)
	FRotator RandomRotation = FRotator::ZeroRotator;

	/** If set to True, the turret will try to find and look at a random rotation. */
	uint8 bCanRotateRandomly : 1 = true;

	/** Used to call FireTurret() in a loop */
	FTimerHandle FireTimer;

	/** FindNewTarget() start this timer to recheck for a new target */
	FTimerHandle SearchTimer;

public:
	virtual void Destroyed() override;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	uint8 bDrawDebug:1 = false;
#endif
};
