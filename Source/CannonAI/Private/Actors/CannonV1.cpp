// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/CannonV1.h"

#include "ACtors/Projectile.h"
#include "Components/Health.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"

ACannonV1::ACannonV1()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	NetUpdateFrequency = 5.0f;
	
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMesh");
	RootComponent = BaseMesh;
	BaseMesh->Mobility = EComponentMobility::Static;
	BaseMesh->bReplicatePhysicsToAutonomousProxy = false;
	BaseMesh->SetGenerateOverlapEvents(false);
	BaseMesh->CanCharacterStepUpOn = ECB_No;

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>("TurretMesh");
	TurretMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
	TurretMesh->bReplicatePhysicsToAutonomousProxy = false;
	TurretMesh->SetGenerateOverlapEvents(false);
	TurretMesh->CanCharacterStepUpOn = ECB_No;
	TurretMesh->SetCanEverAffectNavigation(false);
	
	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>("BarrelMesh");
	BarrelMesh->SetupAttachment(TurretMesh);
	BarrelMesh->bReplicatePhysicsToAutonomousProxy = false;
	BarrelMesh->SetGenerateOverlapEvents(false);
	BarrelMesh->CanCharacterStepUpOn = ECB_No;
	BarrelMesh->SetCanEverAffectNavigation(false);

	Detector = CreateDefaultSubobject<USphereComponent>("DetectorCollision");
	Detector->SetupAttachment(BaseMesh);
	Detector->Mobility = EComponentMobility::Static;
	Detector->SetAreaClassOverride(nullptr);
	Detector->PrimaryComponentTick.bStartWithTickEnabled = false;
	Detector->SetGenerateOverlapEvents(false);	// Enable on the server only
	Detector->CanCharacterStepUpOn = ECB_No;
	Detector->SetCollisionProfileName("Trigger");
	Detector->SetCanEverAffectNavigation(false);

	HealthComp = CreateDefaultSubobject<UHealth>("HealthComponent");

	// Initialize variables
	bCanRotateRandomly = true;
	RandomRotation = FRotator::ZeroRotator;
}

void ACannonV1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ACannonV1, CurrentTarget);
	DOREPLIFETIME(ACannonV1, RandomRotation);
}

void ACannonV1::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		HealthComp->Activate(false);
		
		Detector->SetGenerateOverlapEvents(true);
		Detector->OnComponentBeginOverlap.AddDynamic(this, &ACannonV1::DetectorBeginOverlap);
		Detector->OnComponentEndOverlap.AddDynamic(this, &ACannonV1::DetectorEndOverlap);
	}
}

void ACannonV1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cannon will start rotation randomly if there is no target.
	if (CanRotateRandomly())
	{
		bCanRotateRandomly = false;

		// Delay between switching to a new rotation
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannonV1::FindRandomRotation, 2.0f);
	}

	FRotator NewRotation = CalculateRotation(TurretMesh, DeltaTime);
	TurretMesh->SetRelativeRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

	NewRotation = CalculateRotation(BarrelMesh, DeltaTime);
	BarrelMesh->SetRelativeRotation(FRotator(FMath::ClampAngle(NewRotation.Pitch, CannonInfo.MinPitch, CannonInfo.MaxPitch), 0.0f, 0.0f));
}

void ACannonV1::DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!CurrentTarget)
	{
		FindNewTarget();
	}
}

void ACannonV1::DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == CurrentTarget)
	{
		FindNewTarget();
	}
}

/**
 *	TODO: Cannon should ignore the current target when it goes behind a cover
 *	Possible solution: Use the new CanHitTarget() option (bFullCheck)
 */

void ACannonV1::FindNewTarget()
{
	// Clear the search timer because we are starting a new search
	GetWorld()->GetTimerManager().ClearTimer(SearchTimer);
	
	CurrentTarget = nullptr;
	
	HandleFindNewTarget();

	// Only reset the random rotation if the cannon is not switching between targets.
	if (!CurrentTarget)
	{
		// Start random rotation if failed to find another target.
		bCanRotateRandomly = true;
	}
}

void ACannonV1::HandleFindNewTarget()
{
	TArray<AActor*> Actors;
	Detector->GetOverlappingActors(Actors);

	if (Actors.Num() > 0)
	{
		for (AActor* NewTarget : Actors)
		{
			if (CanHitTarget(NewTarget, false))
			{
				CurrentTarget = NewTarget;
				StartFire();
				return;
			}
		}

		// Retry
		GetWorld()->GetTimerManager().SetTimer(SearchTimer, this, &ACannonV1::HandleFindNewTarget, 0.5f);
	}
}

void ACannonV1::StartFire()
{
	if (CanHitTarget(CurrentTarget, true))
	{
		MulticastFireCannon(CalculateProjectileDirection());
	}
		
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ACannonV1::FireCannon, CannonInfo.FireRate, true);
}

void ACannonV1::FireCannon()
{
	if (CurrentTarget && CanHitTarget(CurrentTarget, true))
	{
		MulticastFireCannon(CalculateProjectileDirection());
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
		FindNewTarget();
	}
}

TArray<FRotator> ACannonV1::CalculateProjectileDirection() const
{
	TArray<FRotator> OutRotations;
	const FRotator SocketRotation = BarrelMesh->GetSocketRotation("ProjectileSocket");
	
	if (CannonInfo.CannonAbility & static_cast<uint32>(ECannonAbility::Shotgun))
	{
		uint8 i = 1;
		while (i <= 3)
		{
			FRotator NewRotation;
			
			NewRotation.Pitch	= SocketRotation.Pitch	+ FMath::RandRange(CannonInfo.AccuracyOffset * -1.0f, CannonInfo.AccuracyOffset);
			NewRotation.Yaw		= SocketRotation.Yaw	+ FMath::RandRange(CannonInfo.AccuracyOffset * -1.0f, CannonInfo.AccuracyOffset);
			NewRotation.Roll	= SocketRotation.Roll	+ FMath::RandRange(CannonInfo.AccuracyOffset * -1.0f, CannonInfo.AccuracyOffset);

			OutRotations.Add(NewRotation);
			++i;
		}
	}
	else
	{
		OutRotations.Add(SocketRotation);
	}

	return OutRotations;
}

bool ACannonV1::CanHitTarget(AActor* Target, bool bUseMuzzle) const
{
	FVector StartLocation, EndLocation;
	
	if (bUseMuzzle)
	{
		StartLocation = BarrelMesh->GetSocketLocation("ProjectileSocket");
		EndLocation = StartLocation + BarrelMesh->GetForwardVector() * (Detector->GetUnscaledSphereRadius() + 100.0f);
	}
	else
	{
		StartLocation = BaseMesh->GetSocketLocation("ConnectionSocket");
		EndLocation = Target->GetActorLocation();
	}
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);	// Ignore self

	// NOTE: For better results, the sphere radius should match the projectile radius
	if (GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECC_Camera,
		FCollisionShape::MakeSphere(50.0f), QueryParams))
	{
		if (HitResult.GetActor() == Target)
		{
			return true;
		}
	}
	return false;
}

void ACannonV1::FindRandomRotation()
{
	if (!CurrentTarget)
	{
		if (FMath::IsNearlyEqual(RandomRotation.Pitch, BarrelMesh->GetRelativeRotation().Pitch, 1))
		{
			RandomRotation = FRotator(FMath::RandRange(CannonInfo.MinPitch, CannonInfo.MaxPitch), FMath::RandRange(-180.0f, 180.0f), 0.0f);
			
			bCanRotateRandomly = true;
		}
		else	// When the barrel hasn't reached the target rotation, retry after a delay
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannonV1::FindRandomRotation, 1.0f);
		}
	}
}

FRotator ACannonV1::CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const
{
	if (CurrentTarget)
	{
		// Follow the target
		const FVector NewLocation = CurrentTarget->GetActorLocation() - BarrelMesh->GetComponentLocation();
		const FRotator TargetRotation = FRotationMatrix::MakeFromX(GetActorTransform().InverseTransformVectorNoScale(NewLocation)).Rotator();	// Inverse Transform Direction
		return FMath::RInterpConstantTo(CompToRotate->GetRelativeRotation(), TargetRotation, DeltaTime, CannonInfo.RotationSpeed);
	}
	
	// Perform random rotation
	return FMath::RInterpConstantTo(CompToRotate->GetRelativeRotation(), RandomRotation, DeltaTime, CannonInfo.RotationSpeed);
}

bool ACannonV1::CanRotateRandomly() const
{
	if (GetLocalRole() == ROLE_Authority && bCanRotateRandomly && !CurrentTarget)
	{
		return true;
	}

	return false;
}

void ACannonV1::MulticastFireCannon_Implementation(const TArray<FRotator>& Rotations)
{
	FTransform NewTransform;
	NewTransform.SetLocation(BarrelMesh->GetSocketLocation("ProjectileSocket"));

	// If there is more than 1 projectile to spawn, lower the scale
	NewTransform.SetScale3D(Rotations.Num() > 1 ? GetActorScale3D() - 1.0f + 0.4f : GetActorScale3D());
	
	for (FRotator NewRotation : Rotations)
	{
		NewTransform.SetRotation(NewRotation.Quaternion());
		
		SpawnProjectile(NewTransform);
	}

	NewTransform = BarrelMesh->GetSocketTransform("MuzzleSocket");
	
	FFXSystemSpawnParameters SpawnParams;
	SpawnParams.WorldContextObject = GetWorld();
	SpawnParams.SystemTemplate = FireParticle;
	SpawnParams.Location = NewTransform.GetLocation();
	SpawnParams.Rotation = NewTransform.GetRotation().Rotator();
	SpawnParams.Scale = GetActorScale3D() + 0.5f;
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FireSound, NewTransform.GetLocation());
}

void ACannonV1::SpawnProjectile(FTransform Transform)
{
	USceneComponent* NewTarget = nullptr;
	if (CannonInfo.CannonAbility & static_cast<uint32>(ECannonAbility::Homing))
	{
		NewTarget = CurrentTarget->GetRootComponent();
	}

	AProjectile* NewProjectile = GetWorld()->SpawnActorDeferred<AProjectile>(Projectile, Transform, this, GetInstigator());

	// Initialize the projectile
	NewProjectile->HomingTarget = NewTarget;
	
	if (CannonInfo.CannonAbility & static_cast<uint32>(ECannonAbility::ExplosiveShot))
	{
		NewProjectile->ProjectileAbility |= Ability_Explosive;
	}

	// Ignoring collisions between barrel and projectile
	BarrelMesh->IgnoreActorWhenMoving(NewProjectile, true);
	
	UGameplayStatics::FinishSpawningActor(NewProjectile, Transform);
}

void ACannonV1::HealthChanged(float NewHealth)
{
	if (NewHealth <= 0.0f)
	{
		DestroyCannon();
	}
}

void ACannonV1::DestroyCannon()
{
	SetLifeSpan(6.0f);
	
	MulticastDestroyCannon();

	// TODO: Disable detector
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	GetWorld()->GetTimerManager().ClearTimer(SearchTimer);
}

void ACannonV1::MulticastDestroyCannon_Implementation()
{
	SetActorTickEnabled(false);

	FFXSystemSpawnParameters SpawnParams;
	SpawnParams.WorldContextObject = GetWorld();
	SpawnParams.SystemTemplate = DestroyParticle;
	SpawnParams.Location = BaseMesh->GetSocketLocation("ConnectionSocket");
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DestroySound, GetActorLocation());

	BaseMesh->SetCanEverAffectNavigation(false);
	BaseMesh->SetMobility(EComponentMobility::Movable);
	BaseMesh->SetCollisionProfileName("Destructible");
	BaseMesh->SetSimulatePhysics(true);

	TurretMesh->SetCollisionProfileName("Destructible");
	TurretMesh->SetSimulatePhysics(true);

	BarrelMesh->SetCollisionProfileName("Destructible");
	BarrelMesh->SetSimulatePhysics(true);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannonV1::StartSink, 4.0f);
}

void ACannonV1::StartSink() const
{
	BaseMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	TurretMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	BarrelMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
