// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerObstacle.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Polar_Bear_Runner.h"
#include "UObject/ConstructorHelpers.h"

ARunnerObstacle::ARunnerObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root is a static mesh so artists can swap it out in Blueprint
	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	SetRootComponent(ObstacleMesh);
	ObstacleMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ObstacleMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultCubeMesh.Succeeded())
	{
		ObstacleMesh->SetStaticMesh(DefaultCubeMesh.Object);
	}

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

void ARunnerObstacle::BeginPlay()
{
	Super::BeginPlay();

	HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerObstacle::OnHitBoxOverlapBegin);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ARunnerObstacle::OnHitBoxOverlapBegin);
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

	const bool bDamageApplied = Runner->RequestDamageFromObstacle(DamageOverride, this);

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

void ARunnerObstacle::OnHitBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                            bool bFromSweep, const FHitResult& SweepResult)
{
	TryDamageActor(OtherActor);
}

