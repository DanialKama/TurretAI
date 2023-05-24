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
		FindRandomRotation();
		
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
	if (HasAuthority() && bCanRotateRandomly && CurrentTarget == nullptr)
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

	if (Projectile)
	{
		ProjectileLoaded = Projectile.Get();
		if (ProjectileLoaded == nullptr)
		{
			Paths.Add(Projectile.ToSoftObjectPath());
		}
	}

	if (FireParticle)
	{
		FireParticleLoaded = FireParticle.Get();
		if (FireParticleLoaded == nullptr)
		{
			Paths.Add(FireParticle.ToSoftObjectPath());
		}
	}

	if (FireSound)
	{
		FireSoundLoaded = FireSound.Get();
		if (FireSoundLoaded == nullptr)
		{
			Paths.Add(FireSound.ToSoftObjectPath());
		}
	}

	if (DestroyParticle)
	{
		DestroyParticleLoaded = DestroyParticle.Get();
		if (DestroyParticleLoaded == nullptr)
		{
			Paths.Add(DestroyParticle.ToSoftObjectPath());
		}
	}

	if (DestroySound)
	{
		DestroySoundLoaded = DestroySound.Get();
		if (DestroySoundLoaded == nullptr)
		{
			Paths.Add(DestroySound.ToSoftObjectPath());
		}
	}

	if (Paths.IsEmpty())
	{
		return;
	}
	
	UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate::CreateWeakLambda(this, [this]
	{
		if (ProjectileLoaded == nullptr)
		{
			ProjectileLoaded = Projectile.Get();
		}
	
		if (FireParticleLoaded == nullptr)
		{
			FireParticleLoaded = Cast<UNiagaraSystem>(FireParticle.Get());
		}
	
		if (FireSoundLoaded == nullptr)
		{
			FireSoundLoaded = Cast<USoundBase>(FireSound.Get());
		}
	
		if (DestroyParticleLoaded == nullptr)
		{
			DestroyParticleLoaded = Cast<UNiagaraSystem>(DestroyParticle.Get());
		}
	
		if (DestroySoundLoaded == nullptr)
		{
			DestroySoundLoaded = Cast<USoundBase>(DestroySound.Get());
		}
	}));
}

void ATurret::DetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CurrentTarget == nullptr)
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
	
	FindNewTargetImpl();

	// Only reset the random rotation if the turret is not switching between targets.
	if (CurrentTarget == nullptr)
	{
		// Start random rotation if failed to find another target.
		bCanRotateRandomly = true;
	}
}

void ATurret::FindNewTargetImpl()
{
	TArray<AActor*> Actors;
	Detector->GetOverlappingActors(Actors);

	if (Actors.IsEmpty())
	{
		return;
	}
	
	for (AActor* NewTarget : Actors)
	{
		if (CanSeeTarget(NewTarget))
		{
			CurrentTarget = NewTarget;
			StartFireTurret();
			return;
		}
	}

	// Retry
	GetWorld()->GetTimerManager().SetTimer(SearchTimer, this, &ATurret::FindNewTargetImpl, 0.5f);
}

void ATurret::StartFireTurret()
{
	if (CanHitTarget(CurrentTarget))
	{
		HandleFireTurret();
	}
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ATurret::FireTurret, TurretInfo.FireRate, true);
}

void ATurret::FireTurret()
{
	if (CurrentTarget && CanHitTarget(CurrentTarget))
	{
		HandleFireTurret();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);
		FindNewTarget();
	}
}

void ATurret::HandleFireTurret()
{
	MulticastFireTurret();
}

void ATurret::MulticastFireTurret_Implementation()
{
	if (CurrentTarget == nullptr)
	{
		return;
	}
	
	SpawnProjectile(BarrelMesh->GetSocketTransform("ProjectileSocket"));
	SpawnFireFX();
}

void ATurret::SpawnProjectile(const FTransform& Transform)
{
	if (ProjectileLoaded == nullptr)
	{
		if (Projectile)
		{
			ProjectileLoaded = Projectile.LoadSynchronous();
		}
		else
		{
			return;
		}
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
			NewProjectile->SetFlag(EProjectileAbility::Explosive);
		}
		
		// Ignoring collisions between barrel and projectile
		BarrelMesh->IgnoreActorWhenMoving(NewProjectile, true);
		
		UGameplayStatics::FinishSpawningActor(NewProjectile, Transform);
	}
}

void ATurret::SpawnFireFX() const
{
	const FTransform NewTransform = BarrelMesh->GetSocketTransform("MuzzleSocket");
	
	FFXSystemSpawnParameters SpawnParams;
	SpawnParams.WorldContextObject = GetWorld();;
	SpawnParams.SystemTemplate = FireParticleLoaded;
	SpawnParams.Location = NewTransform.GetLocation();
	SpawnParams.Rotation = NewTransform.GetRotation().Rotator();
	SpawnParams.Scale = GetActorScale3D() + 0.5f;
	UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams);
	
	UGameplayStatics::SpawnSoundAtLocation(SpawnParams.WorldContextObject, FireSoundLoaded, SpawnParams.Location);
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

bool ATurret::CanSeeTarget(AActor* Target) const
{
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByProfile(HitResult, BaseMesh->GetSocketLocation("ConnectionSocket"), Target->GetActorLocation(), UCollisionProfile::Pawn_ProfileName, CollisionParams))
	{
		return HitResult.GetActor() == Target;
	}
	
	return false;
}

bool ATurret::CanHitTarget(AActor* Target) const
{
	FVector StartLocation = BarrelMesh->GetSocketLocation("ProjectileSocket");
	FVector EndLocation = StartLocation + BarrelMesh->GetForwardVector() * (Detector->GetUnscaledSphereRadius() + 100.0f);
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.MobilityType = EQueryMobilityType::Dynamic;

	// NOTE: For better results, the sphere radius should match the projectile radius
	FHitResult HitResult;
	if (GetWorld()->SweepSingleByProfile(HitResult, StartLocation, EndLocation, FQuat::Identity, UCollisionProfile::Pawn_ProfileName, FCollisionShape::MakeSphere(50.0f), CollisionParams) &&
		HitResult.GetActor() == Target)
	{
		return true;
	}
	
	return false;
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
		
		// Spawn the turret base
		const ADestroyedStructure* NewStructure = Cast<ADestroyedStructure>(MyWorld->SpawnActor(ADestroyedStructure::StaticClass(), &BaseMesh->GetComponentTransform()));
		NewStructure->Initialize(BaseMesh->GetStaticMesh(), BaseMesh->GetMaterials(), BaseMesh->GetLinearDamping(), BaseMesh->GetAngularDamping());
	
		// Spawn the turret barrel
		NewStructure = Cast<ADestroyedStructure>(MyWorld->SpawnActor(ADestroyedStructure::StaticClass(), &BarrelMesh->GetComponentTransform()));
		NewStructure->Initialize(BarrelMesh->GetStaticMesh(), BarrelMesh->GetMaterials(), BarrelMesh->GetLinearDamping(), BarrelMesh->GetAngularDamping());
	}

	Super::Destroyed();
}
