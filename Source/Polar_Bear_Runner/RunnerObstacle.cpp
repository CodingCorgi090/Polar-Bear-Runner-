// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerObstacle.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Polar_Bear_Runner.h"
#include "ApplyWhiteMaterial.h"


/**
namespace
{
void ApplyWhiteMaterial(UStaticMeshComponent* MeshComponent, UObject* Owner)
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	UMaterialInterface* WhiteBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (WhiteBaseMaterial == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* WhiteMaterial = UMaterialInstanceDynamic::Create(WhiteBaseMaterial, Owner);
	if (WhiteMaterial == nullptr)
	{
		return;
	}

	WhiteMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
	WhiteMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
	WhiteMaterial->SetVectorParameterValue(TEXT("Base Color"), FLinearColor::White);
	MeshComponent->SetMaterial(0, WhiteMaterial);
}
}
*/

ARunnerObstacle::ARunnerObstacle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(RootComponent);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ObstacleMesh->SetCollisionObjectType(ECC_WorldDynamic);
	ObstacleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ObstacleMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ObstacleMesh->SetGenerateOverlapEvents(true);

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

	ObstacleLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ObstacleLabel"));
	ObstacleLabel->SetupAttachment(RootComponent);
	ObstacleLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ObstacleLabel->SetGenerateOverlapEvents(false);
	ObstacleLabel->SetHorizontalAlignment(EHTA_Center);
	ObstacleLabel->SetVerticalAlignment(EVRTA_TextCenter);
	ObstacleLabel->SetTextRenderColor(LabelColor);
	ObstacleLabel->SetWorldSize(LabelWorldSize);
	ObstacleLabel->SetRelativeLocation(LabelRelativeLocation);

	ObstacleLabelText = FText::FromString(TEXT("O"));
}

void ARunnerObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ObstacleMeshAsset != nullptr)
	{
		ObstacleMesh->SetStaticMesh(ObstacleMeshAsset);
	}

	ApplyVisualSettings();
	ConfigureLabel();
	UpdateHitBoxFromMeshBounds();
	ConfigureDamageCollision();
}

void ARunnerObstacle::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisualSettings();
	ConfigureLabel();
	UpdateHitBoxFromMeshBounds();
	ConfigureDamageCollision();
	UpdateLabelFacingPlayer();

	HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);

	ObstacleMesh->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
	ObstacleMesh->OnComponentBeginOverlap.AddDynamic(this, &ARunnerObstacle::OnDamageOverlapBegin);
}

void ARunnerObstacle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLabelFacingPlayer();
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

void ARunnerObstacle::RefreshObstacleShape()
{
	SetActorScale3D(FVector::OneVector);
	ApplyVisualSettings();
	ConfigureLabel();
	UpdateHitBoxFromMeshBounds();
	ConfigureDamageCollision();
	UpdateLabelFacingPlayer();
}

void ARunnerObstacle::ApplyVisualSettings()
{
	bMakeObstacleCubeLike = true;

	if (!bMakeObstacleCubeLike || ObstacleMesh == nullptr)
	{
		return;
	}

	SetActorScale3D(FVector::OneVector);
	ObstacleMesh->SetRelativeLocation(FVector::ZeroVector);

	if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		ObstacleMesh->SetStaticMesh(CubeMesh);
	}

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents(StaticMeshComponents);
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (!IsValid(StaticMeshComponent) || StaticMeshComponent == ObstacleMesh)
		{
			continue;
		}

		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetGenerateOverlapEvents(false);
		StaticMeshComponent->SetVisibility(false, true);
		StaticMeshComponent->SetHiddenInGame(true, true);
	}

	const UStaticMesh* StaticMesh = ObstacleMesh->GetStaticMesh();
	if (StaticMesh == nullptr)
	{
		return;
	}

	const FVector MeshSize = StaticMesh->GetBounds().BoxExtent * 2.0f;
	const float CubeSize = FMath::Max3(ObstacleVisualSize.X, ObstacleVisualSize.Y, ObstacleVisualSize.Z);
	const FVector DesiredSize(FMath::Max(CubeSize, 1.0f));

	const FVector NewScale(
		MeshSize.X > UE_KINDA_SMALL_NUMBER ? DesiredSize.X / MeshSize.X : 1.0f,
		MeshSize.Y > UE_KINDA_SMALL_NUMBER ? DesiredSize.Y / MeshSize.Y : 1.0f,
		MeshSize.Z > UE_KINDA_SMALL_NUMBER ? DesiredSize.Z / MeshSize.Z : 1.0f);

	ObstacleMesh->SetRelativeScale3D(NewScale);
	ApplyWhiteMaterial(ObstacleMesh, this);
}

void ARunnerObstacle::ConfigureLabel()
{
	if (ObstacleLabel == nullptr)
	{
		return;
	}

	bLabelFacesPlayer = false;
	ObstacleLabelText = FText::FromString(TEXT("O"));
	ObstacleLabel->SetText(ObstacleLabelText);
	LabelRelativeLocation = FVector(-77.0f, 0.0f, 0.0f);
	ObstacleLabel->SetRelativeLocation(LabelRelativeLocation);
	ObstacleLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	ObstacleLabel->SetWorldSize(LabelWorldSize);
	ObstacleLabel->SetTextRenderColor(LabelColor);
	ObstacleLabel->SetHiddenInGame(!bShowLabel);
	ObstacleLabel->SetVisibility(bShowLabel, true);
}

void ARunnerObstacle::UpdateLabelFacingPlayer()
{
	if (!bShowLabel || !bLabelFacesPlayer || ObstacleLabel == nullptr)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn == nullptr)
	{
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - ObstacleLabel->GetComponentLocation();
	ToPlayer.Z = 0.0f;

	if (!ToPlayer.Normalize())
	{
		return;
	}

	ObstacleLabel->SetWorldRotation(FRotationMatrix::MakeFromX(ToPlayer).Rotator());
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

	if (bMakeObstacleCubeLike)
	{
		const float CubeSize = FMath::Max3(ObstacleVisualSize.X, ObstacleVisualSize.Y, ObstacleVisualSize.Z);
		const float HalfCubeSize = FMath::Max(CubeSize * 0.5f, 1.0f);
		const FVector HalfSize(
			FMath::Max(HalfCubeSize + HitBoxPadding.X, 1.0f),
			FMath::Max(HalfCubeSize + HitBoxPadding.Y, 1.0f),
			FMath::Max(HalfCubeSize + HitBoxPadding.Z, 1.0f));

		HitBox->SetRelativeLocation(FVector::ZeroVector);
		HitBox->SetBoxExtent(HalfSize, true);
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

