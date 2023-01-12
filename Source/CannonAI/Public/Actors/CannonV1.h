// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Cannon.h"
#include "CannonV1.generated.h"

/**
 * This version of the cannon includes a base and a barrel
 */
UCLASS(meta = (DisplayName = "Cannon AI V1"))
class CANNONAI_API ACannonV1 : public ACannon
{
	GENERATED_BODY()

// Functions
protected:
	/** Called every frame */
	virtual void Tick(float DeltaTime) override;
};
