// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/Cannon.h"

#include "ACtors/Projectile.h"
#include "Components/Health.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

ACannon::ACannon()
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
	
	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>("BarrelMesh");
	BarrelMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
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

void ACannon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(ACannon, CurrentTarget);
	DOREPLIFETIME(ACannon, RandomRotation);
}

void ACannon::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		HealthComp->Activate(false);
		
		Detector->SetGenerateOverlapEvents(true);
		Detector->OnComponentBeginOverlap.AddDynamic(this, &ACannon::DetectorBeginOverlap);
		Detector->OnComponentEndOverlap.AddDynamic(this, &ACannon::DetectorEndOverlap);
	}
}

void ACannon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cannon will start rotation randomly if there is no target.
	if (CanRotateRandomly())
	{
		bCanRotateRandomly = false;

		// Delay between switching to a new rotation
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannon::FindRandomRotation, 2.0f);
	}
}

void ACannon::DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!CurrentTarget)
	{
		FindNewTarget();
	}
}

void ACannon::DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == CurrentTarget)
	{
		FindNewTarget();
	}
}

void ACannon::FindNewTarget()
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

void ACannon::HandleFindNewTarget()
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
		GetWorld()->GetTimerManager().SetTimer(SearchTimer, this, &ACannon::HandleFindNewTarget, 0.5f);
	}
}

void ACannon::StartFire()
{
	if (CanHitTarget(CurrentTarget, true))
	{
		MulticastFireCannon(CalculateProjectileDirection());
	}
		
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ACannon::FireCannon, CannonInfo.FireRate, true);
}

void ACannon::FireCannon()
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

TArray<FRotator> ACannon::CalculateProjectileDirection() const
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

bool ACannon::CanHitTarget(AActor* Target, bool bUseMuzzle) const
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

void ACannon::FindRandomRotation()
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
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannon::FindRandomRotation, 1.0f);
		}
	}
}

FRotator ACannon::CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const
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

bool ACannon::CanRotateRandomly() const
{
	if (GetLocalRole() == ROLE_Authority && bCanRotateRandomly && !CurrentTarget)
	{
		return true;
	}

	return false;
}

void ACannon::MulticastFireCannon_Implementation(const TArray<FRotator>& Rotations)
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

void ACannon::SpawnProjectile(FTransform Transform)
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

void ACannon::HealthChanged(float NewHealth)
{
	if (NewHealth <= 0.0f)
	{
		DestroyCannon();
	}
}

void ACannon::DestroyCannon()
{
	SetLifeSpan(6.0f);
	
	MulticastDestroyCannon();

	// TODO: Disable detector
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	GetWorld()->GetTimerManager().ClearTimer(SearchTimer);
}

void ACannon::MulticastDestroyCannon_Implementation()
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

	BarrelMesh->SetCollisionProfileName("Destructible");
	BarrelMesh->SetSimulatePhysics(true);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACannon::StartSink, 4.0f);
}

void ACannon::StartSink() const
{
	BaseMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	BarrelMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
}
