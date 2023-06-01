// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TURRETAI_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

// Functions
public:
	/** Sets default values for this component's properties */
	UHealthComponent();

	virtual void Activate(bool bReset) override;

private:
	UFUNCTION()
	void OwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

// Variables
public:
	UPROPERTY(EditAnywhere, Category = "Default", meta = (ClampMin = 0.0, UIMin = 0.0))
	float DefaultHealth = 100.0f;

	float CurrentHealth = 0.0f;

private:
	TScriptInterface<class IGameplayInterface> OwnerInterface;

	uint8 bIsAlive : 1;
};
