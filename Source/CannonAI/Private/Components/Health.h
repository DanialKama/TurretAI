// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Health.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CANNONAI_API UHealth : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UHealth();

	virtual void Activate(bool bReset) override;

private:
	UFUNCTION()
	void OwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

// Variables
public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DefaultHealth = 100.0f;

private:
	uint8 bIsAlive : 1;

	float CurrentHealth = 0.0f;

	TObjectPtr<class IGameplayInterface> GameplayInterface;
};
