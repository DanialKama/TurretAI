// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/GameplayInterface.h"
#include "Structures/TurretInfo.h"
#include "Turret.generated.h"

class UNiagaraSystem;
class USoundCue;

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
	TObjectPtr<class UHealth> HealthComp;

// Functions
public:
	/** Sets default values for this actor's properties */
	ATurret();

	// Gameplay Interface
	virtual void HealthChanged(float NewHealth) override;
	
protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;
	
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
	
	/** Calculating the target rotation so the turret will smoothly rotate toward the target */
	FRotator CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const;
	
	virtual void MulticastDestroyTurret_Implementation();
	
	virtual void StartSink() const;

private:
	UFUNCTION()
	void DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** On losing the current target, switch to another target if there is any. */
	void FindNewTarget();

	/** Should not be called directly, use FindNewTarget() */
	void HandleFindNewTarget();

	/** Trying to fire the turret based on the current state of the target (enemy). */
	void StartFire();

	/** Fire the weapon */
	void FireWeapon();
	
	/**
	 * Checking the target state and see that can projectile hit the target
	 * @param	Target		Target actor that we try to hit
	 * @param	bUseMuzzle	Should use the muzzle location as the start location for the trace?
	 * @return	TRUE if the projectile can hit the target
	 */
	bool CanHitTarget(AActor* Target, bool bUseMuzzle) const;

	/** Calculating direction for projectiles based on the turret ability and Accuracy Offset */
	TArray<FRotator> CalculateProjectileDirection() const;
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireWeapon(const TArray<FRotator>& Rotations);
	void MulticastFireWeapon_Implementation(const TArray<FRotator>& Rotations);

	void SpawnProjectile(FTransform Transform);

	/** Finding a new random rotation for the turret to use when there is no enemy */
	void FindRandomRotation();

	/** Can calculate random rotation? */
	bool CanRotateRandomly() const;

	void DestroyTurret();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDestroyTurret();
	
// Variables
protected:
	/** Turret info structure that stores the essential data to initialize the turret */
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	FTurretInfo TurretInfo;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TSubclassOf<class AProjectile> Projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiagaraSystem> FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundCue> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiagaraSystem> DestroyParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Turret", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundCue> DestroySound;

	/** The current enemy that the turret try to shoot at it */
	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/** Target rotation that the turret will try to look at when there is no enemy */
	UPROPERTY(Replicated)
	FRotator RandomRotation = FRotator::ZeroRotator;

	/** If set to True, the turret will try to find and look at a random rotation. */
	uint8 bCanRotateRandomly : 1;

	/** Used to call FireTurret() in a loop */
	FTimerHandle FireTimer;

	/** FindNewTarget() start this timer to recheck for a new target */
	FTimerHandle SearchTimer;
};
