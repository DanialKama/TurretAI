// Copyright 2026 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretV2.generated.h"

/**
 * This version of the turret includes a base, a turret, and barrel
 */
UCLASS(Blueprintable, meta = (DisplayName = "Turret AI V2"))
class TURRETAI_API ATurretV2 : public ATurret
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> TurretMesh;

public:
	ATurretV2();

	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
};
