// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/GameplayInterface.h"
#include "Structures/CannonInfo.h"
#include "Cannon.generated.h"

class UNiagaraSystem;
class USoundCue;

/**
 * Cannon AI base class
 */
UCLASS(Abstract, NotBlueprintable, meta = (DisplayName = "Cannon AI"))
class CANNONAI_API ACannon : public AActor, public IGameplayInterface
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
	ACannon();

	// Gameplay Interface
	virtual void HealthChanged(float NewHealth) override;
	
protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;
	
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
	
	/** Calculating the target rotation so the cannon will smoothly rotate toward the target */
	FRotator CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const;
	
	virtual void MulticastDestroyCannon_Implementation();
	
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

	/** Trying to fire the cannon based on the current state of the target (enemy). */
	void StartFire();

	/** Handling firing the cannon */
	void FireCannon();
	
	/**
	 * Checking the target state and see that can projectile hit the target
	 * @param	Target		Target actor that we try to hit
	 * @param	bUseMuzzle	Should use the muzzle location as the start location for the trace?
	 * @return	TRUE if the projectile can hit the target
	 */
	bool CanHitTarget(AActor* Target, bool bUseMuzzle) const;

	/** Calculating direction for projectiles based on the cannon ability and Accuracy Offset */
	TArray<FRotator> CalculateProjectileDirection() const;
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireCannon(const TArray<FRotator>& Rotations);
	void MulticastFireCannon_Implementation(const TArray<FRotator>& Rotations);

	void SpawnProjectile(FTransform Transform);

	/** Finding a new random rotation for the cannon to use when there is no enemy */
	void FindRandomRotation();

	/** Can calculate random rotation? */
	bool CanRotateRandomly() const;

	void DestroyCannon();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDestroyCannon();
	
// Variables
protected:
	/** Cannon info structure that stores the essential data to initialize the cannon */
	UPROPERTY(EditDefaultsOnly, Category = "Cannon")
	FCannonInfo CannonInfo;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Cannon", meta = (AllowPrivateAccess = true))
	TSubclassOf<class AProjectile> Projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiagaraSystem> FireParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundCue> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon", meta = (AllowPrivateAccess = true))
	TObjectPtr<UNiagaraSystem> DestroyParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundCue> DestroySound;

	/** The current enemy that the cannon try to shoot at it */
	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/** Target rotation that the cannon will try to look at when there is no enemy */
	UPROPERTY(Replicated)
	FRotator RandomRotation = FRotator::ZeroRotator;

	/** If set to True, the cannon will try to find and look at a random rotation. */
	uint8 bCanRotateRandomly : 1;

	/** Used to call FireCannon() in a loop */
	FTimerHandle FireTimer;

	/** FindNewTarget() start this timer to recheck for a new target */
	FTimerHandle SearchTimer;
};
