// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UGameplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Gameplay Interface
 */
class IGameplayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** The health component will call this after updating the current health amount */
	virtual void HealthChanged(float NewHealth) {}
};
