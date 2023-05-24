// Copyright 2023 Danial Kamali. All Rights Reserved.

#include "Actors/Turret.h"

#include "Actors/Projectile.h"
#include "Components/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "DestroyedStructure.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	NetUpdateFrequency = 5.0f;
	
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	RootComponent = BaseMesh;
	BaseMesh->Mobility = EComponentMobility::Static;
	BaseMesh->SetGenerateOverlapEvents(false);
	BaseMesh->CanCharacterStepUpOn = ECB_No;
	
	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel Mesh"));
	BarrelMesh->SetupAttachment(BaseMesh, "ConnectionSocket");
	BarrelMesh->SetGenerateOverlapEvents(false);
	BarrelMesh->CanCharacterStepUpOn = ECB_No;
	BarrelMesh->SetCanEverAffectNavigation(false);

	Detector = CreateDefaultSubobject<USphereComponent>(TEXT("Detector Collision"));
	Detector->SetupAttachment(BaseMesh);
	Detector->Mobility = EComponentMobility::Static;
	Detector->SetAreaClassOverride(nullptr);
	Detector->SetGenerateOverlapEvents(false);	// Enable on the server only
	Detector->CanCharacterStepUpOn = ECB_No;
	Detector->SetCollisionProfileName("Trigger");
	Detector->SetCanEverAffectNavigation(false);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));

	// Initialize variables
	bCanRotateRandomly = true;
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurret, CurrentTarget);
	DOREPLIFETIME(ATurret, RandomRotation);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	LoadAssets();

	if (HasAuthority())
	{
		HealthComp->Activate(false);
		
		Detector->SetGenerateOverlapEvents(true);
		Detector->OnComponentBeginOverlap.AddDynamic(this, &ATurret::DetectorBeginOverlap);
		Detector->OnComponentEndOverlap.AddDynamic(this, &ATurret::DetectorEndOverlap);
	}
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Turret will start rotation randomly if there is no target.
	if (HasAuthority() && bCanRotateRandomly && !CurrentTarget)
	{
		bCanRotateRandomly = false;

		// Delay between switching to a new rotation
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATurret::FindRandomRotation, 2.0f);
	}
}

void ATurret::LoadAssets()
{
	TArray<FSoftObjectPath> Paths;

	ProjectileLoaded = Projectile.Get();
	if (!ProjectileLoaded)
	{
		Paths.Add(Projectile.ToSoftObjectPath());
	}
	
	FireParticleLoaded = FireParticle.Get();
	if (!FireParticleLoaded)
	{
		Paths.Add(FireParticle.ToSoftObjectPath());
	}

	FireSoundLoaded = FireSound.Get();
	if (!FireSoundLoaded)
	{
		Paths.Add(FireSound.ToSoftObjectPath());
	}

	DestroyParticleLoaded = DestroyParticle.Get();
	if (!DestroyParticleLoaded)
	{
		Paths.Add(DestroyParticle.ToSoftObjectPath());
	}

	DestroySoundLoaded = DestroySound.Get();
	if (!DestroySoundLoaded)
	{
		Paths.Add(DestroySound.ToSoftObjectPath());
	}

	if (Paths.IsEmpty())
	{
		return;
	}
	
	UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate::CreateWeakLambda(this, [this]
	{
		if (!ProjectileLoaded)
		{
			ProjectileLoaded = Projectile.Get();
		}
	
		if (!FireParticleLoaded)
		{
			FireParticleLoaded = Cast<UNiagaraSystem>(FireParticle.Get());
		}
	
		if (!FireSoundLoaded)
		{
			FireSoundLoaded = Cast<USoundBase>(FireSound.Get());
		}
	
		if (!DestroyParticleLoaded)
		{
			DestroyParticleLoaded = Cast<UNiagaraSystem>(DestroyParticle.Get());
		}
	
		if (!DestroySoundLoaded)
		{
			DestroySoundLoaded = Cast<USoundBase>(DestroySound.Get());
		}
	}));
}

void ATurret::DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!CurrentTarget)
	{
		FindNewTarget();
	}
}

void ATurret::DetectorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == CurrentTarget)
	{
		FindNewTarget();
	}
}

void ATurret::FindNewTarget()
{
	// Clear the search timer because we are starting a new search
	GetWorld()->GetTimerManager().ClearTimer(SearchTimer);
	
	CurrentTarget = nullptr;
	
	HandleFindNewTarget();

	// Only reset the random rotation if the turret is not switching between targets.
	if (!CurrentTarget)
	{
		// Start random rotation if failed to find another target.
		bCanRotateRandomly = true;
	}
}

void ATurret::HandleFindNewTarget()
{
	TArray<AActor*> Actors;
	Detector->GetOverlappingActors(Actors);

	if (Actors.IsEmpty())
	{
		return;
	}
	
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
	GetWorld()->GetTimerManager().SetTimer(SearchTimer, this, &ATurret::HandleFindNewTarget, 0.5f);
}

void ATurret::StartFire()
{
	if (CanHitTarget(CurrentTarget, true))
	{
		MulticastFireWeapon(CalculateProjectileDirection());
	}
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ATurret::FireWeapon, TurretInfo.FireRate, true);
}

void ATurret::FireWeapon()
{
	if (CurrentTarget && CanHitTarget(CurrentTarget, true))
	{
		MulticastFireWeapon(CalculateProjectileDirection());
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
		FindNewTarget();
	}
}

TArray<FRotator> ATurret::CalculateProjectileDirection() const
{
	TArray<FRotator> OutRotations;
	const FRotator SocketRotation = BarrelMesh->GetSocketRotation("ProjectileSocket");
	
	if (TurretInfo.HasFlag(ETurretAbility::Shotgun))
	{
		uint8 i = 1;
		while (i <= 3)
		{
			FRotator NewRotation;
			
			NewRotation.Pitch	= SocketRotation.Pitch	+ FMath::RandRange(TurretInfo.AccuracyOffset * -1.0f, TurretInfo.AccuracyOffset);
			NewRotation.Yaw		= SocketRotation.Yaw	+ FMath::RandRange(TurretInfo.AccuracyOffset * -1.0f, TurretInfo.AccuracyOffset);
			NewRotation.Roll	= SocketRotation.Roll	+ FMath::RandRange(TurretInfo.AccuracyOffset * -1.0f, TurretInfo.AccuracyOffset);

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

bool ATurret::CanHitTarget(AActor* Target, bool bUseMuzzle) const
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
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// NOTE: For better results, the sphere radius should match the projectile radius
	FHitResult HitResult;
	if (GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(50.0f), QueryParams) &&
		HitResult.GetActor() == Target)
	{
		return true;
	}
	
	return false;
}

void ATurret::FindRandomRotation()
{
	if (CurrentTarget)
	{
		return;
	}
	
	if (FMath::IsNearlyEqual(RandomRotation.Pitch, BarrelMesh->GetRelativeRotation().Pitch, 1))
	{
		RandomRotation = FRotator(FMath::RandRange(TurretInfo.MinPitch, TurretInfo.MaxPitch), FMath::RandRange(-180.0f, 180.0f), 0.0f);
			
		bCanRotateRandomly = true;
	}
	else
	{
		// When the barrel hasn't reached the target rotation, retry after a delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ATurret::FindRandomRotation, 1.0f);
	}
}

FRotator ATurret::CalculateRotation(const UStaticMeshComponent* CompToRotate, float DeltaTime) const
{
	if (CurrentTarget)
	{
		// Follow the target
		const FVector NewLocation = CurrentTarget->GetActorLocation() - BarrelMesh->GetComponentLocation();
		const FRotator TargetRotation = FRotationMatrix::MakeFromX(GetActorTransform().InverseTransformVectorNoScale(NewLocation)).Rotator();	// Inverse Transform Direction
		return FMath::RInterpConstantTo(CompToRotate->GetRelativeRotation(), TargetRotation, DeltaTime, TurretInfo.RotationSpeed);
	}
	
	// Perform random rotation
	return FMath::RInterpConstantTo(CompToRotate->GetRelativeRotation(), RandomRotation, DeltaTime, TurretInfo.RotationSpeed);
}

void ATurret::MulticastFireWeapon_Implementation(const TArray<FRotator>& Rotations)
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
	SpawnParams.SystemTemplate = FireParticleLoaded;
	SpawnParams.Location = NewTransform.GetLocation();
	SpawnParams.Rotation = NewTransform.GetRotation().Rotator();
	SpawnParams.Scale = GetActorScale3D() + 0.5f;
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(SpawnParams.WorldContextObject, FireSoundLoaded, SpawnParams.Location);
}

void ATurret::SpawnProjectile(const FTransform& Transform)
{
	if (ProjectileLoaded == nullptr)
	{
		ProjectileLoaded = Projectile.LoadSynchronous();
	}
	
	if (AProjectile* NewProjectile = GetWorld()->SpawnActorDeferred<AProjectile>(ProjectileLoaded, Transform, this, GetInstigator()))
	{
		// Initialize the projectile
		if (TurretInfo.HasFlag(ETurretAbility::Homing))
		{
			NewProjectile->HomingTarget = CurrentTarget->GetRootComponent();
		}
		
		if (TurretInfo.HasFlag(ETurretAbility::ExplosiveShot))
		{
			NewProjectile->ProjectileAbility |= Explosive;
		}
		
		// Ignoring collisions between barrel and projectile
		BarrelMesh->IgnoreActorWhenMoving(NewProjectile, true);
		
		UGameplayStatics::FinishSpawningActor(NewProjectile, Transform);
	}
}

void ATurret::HealthChanged(float NewHealth)
{
	if (NewHealth <= 0.0f)
	{
		Destroy();
	}
}

void ATurret::Destroyed()
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld->HasBegunPlay())
	{
		FFXSystemSpawnParameters SpawnParams;
		SpawnParams.WorldContextObject = MyWorld;
		SpawnParams.SystemTemplate = DestroyParticleLoaded;
		SpawnParams.Location = BaseMesh->GetSocketLocation("ConnectionSocket");
		UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
		
		UGameplayStatics::SpawnSoundAtLocation(MyWorld, DestroySoundLoaded, BaseMesh->GetComponentLocation());
		
		// Spawn the cannon base
		const ADestroyedStructure* NewStructure = Cast<ADestroyedStructure>(MyWorld->SpawnActor(ADestroyedStructure::StaticClass(), &BaseMesh->GetComponentTransform()));
		NewStructure->Initialize(BaseMesh->GetStaticMesh(), BaseMesh->GetMaterials(), BaseMesh->GetLinearDamping(), BaseMesh->GetAngularDamping());
	
		// Spawn the cannon barrel
		NewStructure = Cast<ADestroyedStructure>(MyWorld->SpawnActor(ADestroyedStructure::StaticClass(), &BarrelMesh->GetComponentTransform()));
		NewStructure->Initialize(BarrelMesh->GetStaticMesh(), BarrelMesh->GetMaterials(), BarrelMesh->GetLinearDamping(), BarrelMesh->GetAngularDamping());
	}

	Super::Destroyed();
}
