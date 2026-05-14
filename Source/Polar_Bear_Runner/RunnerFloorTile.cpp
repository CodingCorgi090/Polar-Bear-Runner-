// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerFloorTile.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARunnerFloorTile::ARunnerFloorTile()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FloorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMeshComponent->SetupAttachment(SceneRoot);
	FloorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorMeshComponent->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultCubeMesh.Succeeded())
	{
		FloorMesh = DefaultCubeMesh.Object;
		FloorMeshComponent->SetStaticMesh(FloorMesh);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFloorMaterial(
		TEXT("/Game/Textures/AdobeStock_588030218_Mat.AdobeStock_588030218_Mat"));
	if (DefaultFloorMaterial.Succeeded())
	{
		FloorMaterial = DefaultFloorMaterial.Object;
		FloorMeshComponent->SetMaterial(0, FloorMaterial);
	}
}

void ARunnerFloorTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConfigureFloorMesh();
}

void ARunnerFloorTile::ConfigureFloorMesh()
{
	if (!FloorMeshComponent)
	{
		return;
	}

	FloorMeshComponent->SetStaticMesh(FloorMesh);
	if (FloorMaterial == nullptr)
	{
		FloorMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Textures/AdobeStock_588030218_Mat.AdobeStock_588030218_Mat"));
	}
	if (FloorMaterial)
	{
		FloorMeshComponent->SetMaterial(0, FloorMaterial);
	}
	FloorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorMeshComponent->SetGenerateOverlapEvents(false);
	FloorMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -FMath::Max(TileThickness, 1.0f) * 0.5f));

	if (!bAutoScaleMeshToTile || !FloorMesh)
	{
		return;
	}

	const FVector MeshSize = FloorMesh->GetBounds().BoxExtent * 2.0f;
	const FVector DesiredSize(
		FMath::Max(TileLength, 100.0f),
		FMath::Max(TileWidth, 100.0f),
		FMath::Max(TileThickness, 1.0f));

	FloorMeshComponent->SetRelativeScale3D(FVector(
		MeshSize.X > KINDA_SMALL_NUMBER ? DesiredSize.X / MeshSize.X : 1.0f,
		MeshSize.Y > KINDA_SMALL_NUMBER ? DesiredSize.Y / MeshSize.Y : 1.0f,
		MeshSize.Z > KINDA_SMALL_NUMBER ? DesiredSize.Z / MeshSize.Z : 1.0f));
}
