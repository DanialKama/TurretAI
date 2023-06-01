// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Components/HealthComponent.h"

#include "Interfaces/GameplayInterface.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Initialize variables
	bIsAlive = true;
	CurrentHealth = DefaultHealth;
}

void UHealthComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	if (IGameplayInterface* GameplayInterface = Cast<IGameplayInterface>(GetOwner()))
	{
		OwnerInterface.SetObject(GetOwner());
		OwnerInterface.SetInterface(GameplayInterface);
		
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OwnerTakeDamage);
	}
}

void UHealthComponent::OwnerTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsAlive == false)
	{
		return;
	}
	
	CurrentHealth -= Damage;
	
	bIsAlive = CurrentHealth > 0.0f;

	OwnerInterface->HealthChanged();
}
