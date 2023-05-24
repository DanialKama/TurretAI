// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/GameplayInterface.h"
#include "Types/TurretTypes.h"
#include "Turret.generated.h"

class UNiagaraSystem;

/**
 * Turret AI base class
 */
UCLASS(Abstract, NotBlueprintable, meta = (DisplayName = "Turret AI"))
class TURRETAI_API ATurret : public AActor, public IGameplayInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BaseMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class USphereComponent> Detector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UHealthComponent> HealthComp;

// Functions
public:
	/** Sets default values for this actor's properties */
	ATurret();
	
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	//~ Begin Gameplay Interface
	virtual void HealthChanged(float NewHealth) override;
	//~ End Gameplay Interface

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;
	
	virtual void HandleFireTurret();

	void SpawnProjectile(const FTransform& Transform);

	void SpawnFireFX() const;
	
	/** Calculating the target rotation so the turret will smoothly rotate toward the target */
	FRotator CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const;
	
private:
	void LoadAssets();
	
	UFUNCTION()
	void DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** On losing the current target, switch to another target if there is any. */
	void FindNewTarget();

	/** @note Should not be called directly, use FindNewTarget() */
	void FindNewTargetImpl();

	/** A simple test to make sure that the turret can see the target and target is not behind any cover */
	bool CanSeeTarget(AActor* Target) const;

	/**
	* Checking the target state and see that can projectile hit the target
	* @param	Target	Target actor that we try to hit
	* @return	True if the projectile can hit the target
	*/
	virtual bool CanHitTarget(AActor* Target) const;

	/** Trying to fire the turret based on the current state of the target (enemy). */
	void StartFireTurret();

	/** Handling firing the turret */
	void FireTurret();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireTurret();
	void MulticastFireTurret_Implementation();

	/** Finding a new random rotation for the turret to use when there is no enemy */
	void FindRandomRotation();
	
// Variables
protected:
	/** Turret info structure that stores the essential data to initialize the turret */
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	FTurretInfo TurretInfo;

	/** The current enemy that the turret try to shoot at it */
	UPROPERTY(Replicated)
	AActor* CurrentTarget;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSoftClassPtr<class AProjectile> Projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UNiagaraSystem> FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<UNiagaraSystem> DestroyParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSoftObjectPtr<USoundBase> DestroySound;

	/** Target rotation that the turret will try to look at when there is no enemy */
	UPROPERTY(Replicated)
	FRotator RandomRotation = FRotator::ZeroRotator;

	UPROPERTY()
	UClass* ProjectileLoaded;
	
	UPROPERTY()
	UNiagaraSystem* FireParticleLoaded;
	
	UPROPERTY()
	USoundBase* FireSoundLoaded;
	
	UPROPERTY()
	UNiagaraSystem* DestroyParticleLoaded;

	UPROPERTY()
	USoundBase* DestroySoundLoaded;

	/** If set to True, the turret will try to find and look at a random rotation. */
	uint8 bCanRotateRandomly : 1;

	/** Used to call FireTurret() in a loop */
	FTimerHandle FireTimer;

	/** FindNewTarget() start this timer to recheck for a new target */
	FTimerHandle SearchTimer;
};
