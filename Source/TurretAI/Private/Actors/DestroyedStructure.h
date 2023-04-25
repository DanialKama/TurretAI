// Copyright 2023 Danial Kamali. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestroyedStructure.generated.h"

UCLASS()
class TURRETAI_API ADestroyedStructure : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMesh;

// Functions
public:
	ADestroyedStructure();

	void Initialize(UStaticMesh* InMesh, TArray<UMaterialInterface*> Materials, const float InLinearDamping, const float InAngularDamping) const;

private:
	UFUNCTION()
	void MeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void StartSink() const;

// Variables
private:
	uint8 bDoOnceHit : 1;
};
