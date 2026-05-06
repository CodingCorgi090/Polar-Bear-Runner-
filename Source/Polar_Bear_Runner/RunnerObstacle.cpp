// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerObstacle.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Polar_Bear_Runner.h"

ARunnerObstacle::ARunnerObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root is a static mesh with no C++ default asset so Blueprints can choose the visual.
	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	SetRootComponent(ObstacleMesh);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ObstacleMesh->SetCollisionObjectType(ECC_WorldDynamic);
	ObstacleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ObstacleMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ObstacleMesh->SetGenerateOverlapEvents(true);

	// Separate hit box so the damage volume can be tuned independently of the visual mesh
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(RootComponent);
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitBox->SetCollisionObjectType(ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitBox->SetGenerateOverlapEvents(true);
	HitBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));
}

void ARunnerObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ObstacleMeshAsset != nullptr)
	{
		ObstacleMesh->SetStaticMesh(ObstacleMeshAsset);
	}

	UpdateHitBoxFromMeshBounds();
	ConfigureDamageCollision();
}

void ARunnerObstacle::BeginPlay()
{
	Super::BeginPlay();

	UpdateHitBoxFromMeshBounds();
	ConfigureDamageCollision();

	HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);

	ObstacleMesh->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
	ObstacleMesh->OnComponentBeginOverlap.AddDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
}

void ARunnerObstacle::ConfigureDamageCollision()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitBox->SetGenerateOverlapEvents(true);

	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ObstacleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ObstacleMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ObstacleMesh->SetGenerateOverlapEvents(true);
}

void ARunnerObstacle::UpdateHitBoxFromMeshBounds()
{
	if (!bAutoSizeHitBoxToMesh || ObstacleMesh == nullptr || HitBox == nullptr)
	{
		return;
	}

	const UStaticMesh* StaticMesh = ObstacleMesh->GetStaticMesh();
	if (StaticMesh == nullptr)
	{
		return;
	}

	const FBoxSphereBounds MeshBounds = StaticMesh->GetBounds();
	const FVector PaddedExtent(
		FMath::Max(MeshBounds.BoxExtent.X + HitBoxPadding.X, 1.0f),
		FMath::Max(MeshBounds.BoxExtent.Y + HitBoxPadding.Y, 1.0f),
		FMath::Max(MeshBounds.BoxExtent.Z + HitBoxPadding.Z, 1.0f));

	HitBox->SetRelativeLocation(MeshBounds.Origin);
	HitBox->SetBoxExtent(PaddedExtent, true);
}

bool ARunnerObstacle::TryDamageActor(AActor* OtherActor)
{
	if (!bIsActive || !bDamagePlayerOnOverlap)
	{
		return false;
	}

	APolar_Bear_RunnerCharacter* Runner = Cast<APolar_Bear_RunnerCharacter>(OtherActor);
	if (Runner == nullptr)
	{
		return false;
	}

	const bool bDamageApplied = Runner->KillRunner(ERunnerDamageType::ObstacleHit, this);

	if (bDamageApplied)
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("RunnerObstacle '%s' hit player '%s'."),
		       *GetNameSafe(this), *GetNameSafe(Runner));

		BP_OnPlayerHit(Runner);

		if (bDestroyOnHit)
		{
			Destroy();
		}
	}
	else
	{
		BP_OnPlayerHitBlocked(Runner);
	}

	return bDamageApplied;
}

void ARunnerObstacle::OnDamageOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                           bool bFromSweep, const FHitResult& SweepResult)
{
	TryDamageActor(OtherActor);
}

