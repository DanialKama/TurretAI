// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Components/HealthComponent.h"

#include "Interfaces/GameplayInterface.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Initialize variables
	bIsAlive = true;
	CurrentHealth = DefaultHealth;
}

void UHealthComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	GameplayInterface = Cast<IGameplayInterface>(GetOwner());

	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OwnerTakeDamage);
}

void UHealthComponent::OwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsAlive)
	{
		const float NewHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, DefaultHealth);
		
		bIsAlive = NewHealth > 0.0f;

		GameplayInterface->HealthChanged(NewHealth);
	}
}
