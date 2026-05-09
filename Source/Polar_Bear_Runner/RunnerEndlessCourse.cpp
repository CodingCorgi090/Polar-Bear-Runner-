// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerEndlessCourse.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "RunnerFloorTile.h"
#include "RunnerKeyPickup.h"
#include "RunnerObstacle.h"
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

inline void ApplyWhiteMaterial(UStaticMeshComponent* MeshComponent, UObject* Owner)
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

ARunnerEndlessCourse::ARunnerEndlessCourse()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EnsureRuntimeDefaults();
}

void ARunnerEndlessCourse::BeginPlay()
{
	Super::BeginPlay();

	RebuildCourse();
}

void ARunnerEndlessCourse::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearCourse();
	Super::EndPlay(EndPlayReason);
}

void ARunnerEndlessCourse::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RecycleTilesIfNeeded();
}

void ARunnerEndlessCourse::RebuildCourse()
{
	ClearCourse();
	EnsureRuntimeDefaults();
	ApplyFloorDimensionsFromClass();
	HideAuthoredStaticMeshComponents();
	HidePrefabActors();
	SetActorScale3D(FVector::OneVector);
	CourseLocalOrigin = FVector::ZeroVector;

	if (bSnapToFollowTargetOnRebuild)
	{
		SnapCourseToFollowTarget();
	}

	RandomStream.Initialize(bUseRandomSeed ? RandomSeed : FMath::Rand());
	CreateInitialTiles();
}

void ARunnerEndlessCourse::CreateInitialTiles()
{
	const int32 ClampedTileCount = FMath::Max(TileCount, 3);
	NextTileNumber = 0;

	for (int32 TileIndex = 0; TileIndex < ClampedTileCount; ++TileIndex)
	{
		CreateTile(NextTileNumber++);
	}
}

void ARunnerEndlessCourse::CreateTile(int32 TileNumber)
{
	FRunnerCourseTile Tile;

	Tile.FloorCollision = NewObject<UBoxComponent>(this);
	Tile.FloorCollision->SetMobility(EComponentMobility::Movable);
	Tile.FloorCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Tile.FloorCollision->SetCollisionObjectType(ECC_WorldStatic);
	Tile.FloorCollision->SetCollisionResponseToAllChannels(ECR_Block);
	Tile.FloorCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Tile.FloorCollision->SetGenerateOverlapEvents(false);
	Tile.FloorCollision->SetHiddenInGame(true);
	Tile.FloorCollision->SetVisibility(false);
	Tile.FloorCollision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Tile.FloorCollision->RegisterComponent();
	Tile.FloorCollision->SetAbsolute(false, false, true);
	AddInstanceComponent(Tile.FloorCollision);

	Tile.LeftEdgeBlocker = NewObject<UBoxComponent>(this);
	Tile.LeftEdgeBlocker->SetMobility(EComponentMobility::Movable);
	Tile.LeftEdgeBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Tile.LeftEdgeBlocker->SetCollisionObjectType(ECC_WorldStatic);
	Tile.LeftEdgeBlocker->SetCollisionResponseToAllChannels(ECR_Ignore);
	Tile.LeftEdgeBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Tile.LeftEdgeBlocker->CanCharacterStepUpOn = ECB_No;
	Tile.LeftEdgeBlocker->SetGenerateOverlapEvents(false);
	Tile.LeftEdgeBlocker->SetHiddenInGame(true);
	Tile.LeftEdgeBlocker->SetVisibility(false);
	Tile.LeftEdgeBlocker->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Tile.LeftEdgeBlocker->RegisterComponent();
	Tile.LeftEdgeBlocker->SetAbsolute(false, false, true);
	AddInstanceComponent(Tile.LeftEdgeBlocker);

	Tile.RightEdgeBlocker = NewObject<UBoxComponent>(this);
	Tile.RightEdgeBlocker->SetMobility(EComponentMobility::Movable);
	Tile.RightEdgeBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Tile.RightEdgeBlocker->SetCollisionObjectType(ECC_WorldStatic);
	Tile.RightEdgeBlocker->SetCollisionResponseToAllChannels(ECR_Ignore);
	Tile.RightEdgeBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Tile.RightEdgeBlocker->CanCharacterStepUpOn = ECB_No;
	Tile.RightEdgeBlocker->SetGenerateOverlapEvents(false);
	Tile.RightEdgeBlocker->SetHiddenInGame(true);
	Tile.RightEdgeBlocker->SetVisibility(false);
	Tile.RightEdgeBlocker->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Tile.RightEdgeBlocker->RegisterComponent();
	Tile.RightEdgeBlocker->SetAbsolute(false, false, true);
	AddInstanceComponent(Tile.RightEdgeBlocker);

	ConfigureTile(Tile, TileNumber);
	Tiles.Add(Tile);
}

void ARunnerEndlessCourse::ConfigureTile(FRunnerCourseTile& Tile, int32 TileNumber)
{
	const float ClampedTileLength = FMath::Max(TileLength, 100.0f);
	const float ClampedTileWidth = FMath::Max(TileWidth, 1600.0f);
	const float ClampedFloorThickness = FMath::Max(FloorThickness, 1.0f);
	const float CenterX = TileNumber * ClampedTileLength;
	const FVector TileLocalOrigin = CourseLocalOrigin + FVector(CenterX, 0.0f, 0.0f);

	Tile.TileNumber = TileNumber;
	Tile.CenterX = CenterX;

	ConfigureFloorActor(Tile);

	if (Tile.FloorCollision)
	{
		const FVector FloorCollisionWorldLocation = GetActorTransform().TransformPosition(TileLocalOrigin + FVector(0.0f, 0.0f, -ClampedFloorThickness * 0.5f));
		Tile.FloorCollision->SetWorldLocationAndRotation(FloorCollisionWorldLocation, GetActorRotation());
		Tile.FloorCollision->SetWorldScale3D(FVector::OneVector);
		Tile.FloorCollision->SetBoxExtent(FVector(ClampedTileLength * 0.5f, ClampedTileWidth * 0.5f, ClampedFloorThickness * 0.5f), true);
	}

	constexpr float WallBelowFloorDepth = 100.0f;
	const float ClampedWallThickness = FMath::Max(EdgeWallThickness, 200.0f);
	const float ClampedWallHeight = FMath::Max(EdgeWallHeight, 5000.0f);
	const FVector WallExtent(ClampedTileLength * 0.5f, ClampedWallThickness * 0.5f, (ClampedWallHeight + WallBelowFloorDepth) * 0.5f);
	const float EdgeOffset = ClampedTileWidth * 0.5f + ClampedWallThickness * 0.5f;
	const float WallCenterZ = (ClampedWallHeight - WallBelowFloorDepth) * 0.5f;

	if (Tile.LeftEdgeBlocker)
	{
		const FVector LeftWallWorldLocation = GetActorTransform().TransformPosition(TileLocalOrigin + FVector(0.0f, -EdgeOffset, WallCenterZ));
		Tile.LeftEdgeBlocker->SetWorldLocationAndRotation(LeftWallWorldLocation, GetActorRotation());
		Tile.LeftEdgeBlocker->SetWorldScale3D(FVector::OneVector);
		Tile.LeftEdgeBlocker->SetBoxExtent(WallExtent, true);
	}

	if (Tile.RightEdgeBlocker)
	{
		const FVector RightWallWorldLocation = GetActorTransform().TransformPosition(TileLocalOrigin + FVector(0.0f, EdgeOffset, WallCenterZ));
		Tile.RightEdgeBlocker->SetWorldLocationAndRotation(RightWallWorldLocation, GetActorRotation());
		Tile.RightEdgeBlocker->SetWorldScale3D(FVector::OneVector);
		Tile.RightEdgeBlocker->SetBoxExtent(WallExtent, true);
	}

	SpawnContentsForTile(Tile);
}

void ARunnerEndlessCourse::ClearTileContents(FRunnerCourseTile& Tile)
{
	for (TObjectPtr<AActor>& SpawnedActor : Tile.SpawnedActors)
	{
		if (IsValid(SpawnedActor))
		{
			SpawnedActor->Destroy();
		}
	}

	Tile.SpawnedActors.Reset();
}

void ARunnerEndlessCourse::ClearCourse()
{
	for (FRunnerCourseTile& Tile : Tiles)
	{
		ClearTileContents(Tile);

		if (Tile.FloorActor)
		{
			Tile.FloorActor->Destroy();
		}

		if (Tile.FloorCollision)
		{
			Tile.FloorCollision->DestroyComponent();
		}

		if (Tile.LeftEdgeBlocker)
		{
			Tile.LeftEdgeBlocker->DestroyComponent();
		}

		if (Tile.RightEdgeBlocker)
		{
			Tile.RightEdgeBlocker->DestroyComponent();
		}
	}

	Tiles.Reset();
}

void ARunnerEndlessCourse::RecycleTilesIfNeeded()
{
	AActor* Target = GetFollowTarget();
	if (!Target || Tiles.Num() == 0)
	{
		return;
	}

	if (const APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(Target))
	{
		if (RunnerCharacter->IsRunnerDead())
		{
			return;
		}
	}

	const float LocalTargetX = GetLocalFollowTargetX();
	const float HalfLength = FMath::Max(TileLength, 100.0f) * 0.5f;
	const float BehindDistance = FMath::Max(RecycleBehindDistance, 0.0f);

	while (Tiles.Num() > 0 && Tiles[0].CenterX + HalfLength < LocalTargetX - BehindDistance)
	{
		FRunnerCourseTile RecycledTile = Tiles[0];
		Tiles.RemoveAt(0);

		ClearTileContents(RecycledTile);
		ConfigureTile(RecycledTile, NextTileNumber++);
		Tiles.Add(RecycledTile);
	}
}

void ARunnerEndlessCourse::SpawnContentsForTile(FRunnerCourseTile& Tile)
{
	const int32 ClampedRows = FMath::Max(SpawnRowsPerTile, 1);
	const float ClampedTileLength = FMath::Max(TileLength, 100.0f);

	for (int32 Row = 0; Row < ClampedRows; ++Row)
	{
		const float RowAlpha = static_cast<float>(Row + 1) / static_cast<float>(ClampedRows + 1);
		const float LocalX = Tile.CenterX - ClampedTileLength * 0.5f + RowAlpha * ClampedTileLength;
		const bool bFarEnoughAhead = LocalX >= GetLocalFollowTargetX() + MinSpawnDistanceAhead;
		const bool bAllowSpawns = Tile.TileNumber >= SafeStartTiles && bFarEnoughAhead;

		if (!bAllowSpawns)
		{
			continue;
		}

		if (HasRandomFloorItems())
		{
			if (RandomStream.FRand() <= RandomFloorItemSpawnChance)
			{
				if (const FRunnerRandomFloorItem* RandomItem = ChooseRandomFloorItem())
				{
					const int32 ItemLane = RandomStream.RandRange(0, FMath::Max(LaneCount - 1, 0));
					SpawnActorOnTile(Tile, GetRandomFloorItemClass(*RandomItem), RandomItem->ItemPrefab, LocalX, ItemLane, RandomItem->ZOffset);
				}
			}
			continue;
		}

		int32 ObstacleLane = INDEX_NONE;
		if ((ObstacleClass || ObstaclePrefab) && RandomStream.FRand() <= ObstacleSpawnChance)
		{
			ObstacleLane = RandomStream.RandRange(0, FMath::Max(LaneCount - 1, 0));
			SpawnActorOnTile(Tile, ObstacleClass, ObstaclePrefab, LocalX, ObstacleLane, ObstacleZOffset);
		}

		if ((KeyPickupClass || KeyPickupPrefab) && RandomStream.FRand() <= KeySpawnChance)
		{
			int32 KeyLane = RandomStream.RandRange(0, FMath::Max(LaneCount - 1, 0));
			if (LaneCount > 1 && KeyLane == ObstacleLane)
			{
				KeyLane = (KeyLane + 1) % LaneCount;
			}

			SpawnActorOnTile(Tile, KeyPickupClass, KeyPickupPrefab, LocalX, KeyLane, KeyZOffset);
		}
	}
}

void ARunnerEndlessCourse::SpawnActorOnTile(FRunnerCourseTile& Tile, TSubclassOf<AActor> ActorClass, AActor* ActorPrefab, float LocalX, int32 LaneIndex, float ZOffset)
{
	if ((!ActorClass && !ActorPrefab) || !GetWorld())
	{
		return;
	}

	const FVector LocalLocation = CourseLocalOrigin + FVector(LocalX, GetLaneOffset(LaneIndex), 0.0f);
	const FVector WorldLocation = GetActorTransform().TransformPosition(LocalLocation);
	const FRotator WorldRotation = GetActorRotation();

	if (AActor* SpawnedActor = SpawnFromClassOrPrefab(ActorClass, ActorPrefab, WorldLocation, WorldRotation))
	{
		float EffectiveZOffset = FMath::Max(ZOffset, 0.0f);
		UPrimitiveComponent* PlacementBoundsComponent = nullptr;

		if (ARunnerKeyPickup* RunnerKey = Cast<ARunnerKeyPickup>(SpawnedActor))
		{
			RunnerKey->RefreshKeyShape();
			EffectiveZOffset = 0.0f;
			PlacementBoundsComponent = RunnerKey->GetKeyMesh();
		}
		else if (ARunnerObstacle* RunnerObstacle = Cast<ARunnerObstacle>(SpawnedActor))
		{
			RunnerObstacle->RefreshObstacleShape();
			EffectiveZOffset = FMath::Max(EffectiveZOffset, ObstacleGroundClearance);
			PlacementBoundsComponent = RunnerObstacle->GetObstacleMesh();
		}
		else
		{
			SpawnedActor->SetActorScale3D(FVector::OneVector);

			TArray<UStaticMeshComponent*> MeshComponents;
			SpawnedActor->GetComponents(MeshComponents);

			UStaticMeshComponent* MainMeshComponent = nullptr;
			float LargestMeshVolume = -1.0f;
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!IsValid(MeshComponent) || MeshComponent->GetStaticMesh() == nullptr)
				{
					continue;
				}

				const FVector MeshSize = MeshComponent->GetStaticMesh()->GetBounds().BoxExtent * 2.0f;
				const FVector ComponentScale = MeshComponent->GetRelativeScale3D().GetAbs();
				const float MeshVolume = MeshSize.X * ComponentScale.X * MeshSize.Y * ComponentScale.Y * MeshSize.Z * ComponentScale.Z;
				if (MeshVolume > LargestMeshVolume)
				{
					LargestMeshVolume = MeshVolume;
					MainMeshComponent = MeshComponent;
				}
			}

			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!IsValid(MeshComponent))
				{
					continue;
				}

				if (MeshComponent != MainMeshComponent)
				{
					MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					MeshComponent->SetGenerateOverlapEvents(false);
					MeshComponent->SetVisibility(false, true);
					MeshComponent->SetHiddenInGame(true, true);
					continue;
				}

				if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
				{
					MeshComponent->SetStaticMesh(CubeMesh);
				}

				const FVector MeshSize = MeshComponent->GetStaticMesh()->GetBounds().BoxExtent * 2.0f;
				const float CubeSize = 150.0f;

				MeshComponent->SetRelativeLocation(FVector::ZeroVector);
				MeshComponent->SetRelativeScale3D(FVector(
					MeshSize.X > UE_KINDA_SMALL_NUMBER ? CubeSize / MeshSize.X : 1.0f,
					MeshSize.Y > UE_KINDA_SMALL_NUMBER ? CubeSize / MeshSize.Y : 1.0f,
					MeshSize.Z > UE_KINDA_SMALL_NUMBER ? CubeSize / MeshSize.Z : 1.0f));
				ApplyWhiteMaterial(MeshComponent, SpawnedActor);
				MeshComponent->SetVisibility(true, true);
				MeshComponent->SetHiddenInGame(false, true);
				PlacementBoundsComponent = MeshComponent;

				break;
			}

			TArray<UTextRenderComponent*> ExistingLabels;
			SpawnedActor->GetComponents(ExistingLabels);
			for (UTextRenderComponent* ExistingLabel : ExistingLabels)
			{
				if (IsValid(ExistingLabel))
				{
					ExistingLabel->SetVisibility(false, true);
					ExistingLabel->SetHiddenInGame(true, true);
				}
			}

			if (USceneComponent* SpawnedRoot = SpawnedActor->GetRootComponent())
			{
				UTextRenderComponent* ObstacleLabel = NewObject<UTextRenderComponent>(SpawnedActor, TEXT("GeneratedObstacleLabel"));
				if (ObstacleLabel)
				{
					ObstacleLabel->SetupAttachment(SpawnedRoot);
					ObstacleLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					ObstacleLabel->SetGenerateOverlapEvents(false);
					ObstacleLabel->SetHorizontalAlignment(EHTA_Center);
					ObstacleLabel->SetVerticalAlignment(EVRTA_TextCenter);
					ObstacleLabel->SetText(FText::FromString(TEXT("O")));
					ObstacleLabel->SetTextRenderColor(FColor(230, 80, 50));
					ObstacleLabel->SetWorldSize(55.0f);
					ObstacleLabel->SetRelativeLocation(FVector(-77.0f, 0.0f, 0.0f));
					ObstacleLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
					SpawnedActor->AddInstanceComponent(ObstacleLabel);
					ObstacleLabel->RegisterComponent();
				}
			}

			EffectiveZOffset = FMath::Max(EffectiveZOffset, ObstacleGroundClearance);
		}

		FVector SpawnBoundsOrigin;
		FVector SpawnBoundsExtent;
		if (PlacementBoundsComponent != nullptr)
		{
			const FBoxSphereBounds ComponentBounds = PlacementBoundsComponent->Bounds;
			SpawnBoundsOrigin = ComponentBounds.Origin;
			SpawnBoundsExtent = ComponentBounds.BoxExtent;
		}
		else
		{
			SpawnedActor->GetActorBounds(false, SpawnBoundsOrigin, SpawnBoundsExtent);
		}

		const float FloorWorldZ = GetActorTransform().TransformPosition(CourseLocalOrigin).Z;
		const float CurrentBottomZ = SpawnBoundsOrigin.Z - SpawnBoundsExtent.Z;
		const float DesiredBottomZ = FloorWorldZ + EffectiveZOffset;
		SpawnedActor->AddActorWorldOffset(FVector(0.0f, 0.0f, DesiredBottomZ - CurrentBottomZ), false, nullptr, ETeleportType::TeleportPhysics);

		Tile.SpawnedActors.Add(SpawnedActor);
	}
}

void ARunnerEndlessCourse::EnsureRuntimeDefaults()
{
	FollowActor = nullptr;

	if (!FloorTileClass)
	{
		FloorTileClass = ARunnerFloorTile::StaticClass();
	}

	TileCount = FMath::Max(TileCount, 3);
	EdgeWallHeight = FMath::Max(EdgeWallHeight, 50.0f);
	EdgeWallThickness = FMath::Max(EdgeWallThickness, 1.0f);
	RecycleBehindDistance = FMath::Max(RecycleBehindDistance, 0.0f);

	SpawnRowsPerTile = FMath::Max(SpawnRowsPerTile, 1);
	LaneCount = FMath::Max(LaneCount, 1);
	LaneSpacing = FMath::Max(LaneSpacing, 1.0f);
	KeySpawnChance = FMath::Clamp(KeySpawnChance, 0.0f, 1.0f);
	ObstacleSpawnChance = FMath::Clamp(ObstacleSpawnChance, 0.0f, 1.0f);
	RandomFloorItemSpawnChance = FMath::Clamp(RandomFloorItemSpawnChance, 0.0f, 1.0f);
	SafeStartTiles = FMath::Max(SafeStartTiles, 0);
	MinSpawnDistanceAhead = FMath::Max(MinSpawnDistanceAhead, 0.0f);
	ObstacleGroundClearance = FMath::Max(ObstacleGroundClearance, 0.0f);
}

void ARunnerEndlessCourse::ApplyFloorDimensionsFromClass()
{
	if (FloorTilePrefab)
	{
		TileLength = FMath::Max(FloorTilePrefab->GetTileLength(), 100.0f);
		TileWidth = FMath::Max(FloorTilePrefab->GetTileWidth(), 100.0f);
		FloorThickness = FMath::Max(FloorTilePrefab->GetTileThickness(), 1.0f);
		return;
	}

	const TSubclassOf<ARunnerFloorTile> TileClassToMeasure = FloorTileClass;
	if (!TileClassToMeasure)
	{
		return;
	}

	const ARunnerFloorTile* FloorTileDefault = TileClassToMeasure->GetDefaultObject<ARunnerFloorTile>();
	if (!FloorTileDefault)
	{
		return;
	}

	TileLength = FMath::Max(FloorTileDefault->GetTileLength(), 100.0f);
	TileWidth = FMath::Max(FloorTileDefault->GetTileWidth(), 100.0f);
	FloorThickness = FMath::Max(FloorTileDefault->GetTileThickness(), 1.0f);
}

void ARunnerEndlessCourse::ConfigureFloorActor(FRunnerCourseTile& Tile)
{
	if ((!FloorTileClass && !FloorTilePrefab) || !GetWorld())
	{
		return;
	}

	const FVector LocalLocation = CourseLocalOrigin + FVector(Tile.CenterX, 0.0f, 0.0f);
	const FVector WorldLocation = GetActorTransform().TransformPosition(LocalLocation);
	const FRotator WorldRotation = GetActorRotation();

	if (!IsValid(Tile.FloorActor))
	{
		Tile.FloorActor = SpawnFromClassOrPrefab(FloorTileClass, FloorTilePrefab, WorldLocation, WorldRotation);
	}
	else
	{
		Tile.FloorActor->SetActorLocationAndRotation(WorldLocation, WorldRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (Tile.FloorActor)
	{
		Tile.FloorActor->SetActorScale3D(FVector::OneVector);
	}
}

void ARunnerEndlessCourse::HideAuthoredStaticMeshComponents()
{
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents(StaticMeshComponents);

	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (!IsValid(StaticMeshComponent))
		{
			continue;
		}

		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetVisibility(false, true);
		StaticMeshComponent->SetHiddenInGame(true, true);
	}
}

void ARunnerEndlessCourse::HidePrefabActors()
{
	TArray<AActor*> PrefabActors;
	PrefabActors.Add(FloorTilePrefab.Get());
	PrefabActors.Add(KeyPickupPrefab.Get());
	PrefabActors.Add(ObstaclePrefab.Get());
	for (const FRunnerRandomFloorItem& RandomFloorItem : RandomFloorItems)
	{
		PrefabActors.Add(RandomFloorItem.ItemPrefab.Get());
	}

	for (AActor* PrefabActor : PrefabActors)
	{
		if (!IsValid(PrefabActor))
		{
			continue;
		}

		PrefabActor->SetActorHiddenInGame(true);
		PrefabActor->SetActorEnableCollision(false);
	}
}

void ARunnerEndlessCourse::SnapCourseToFollowTarget()
{
	AActor* Target = GetFollowTarget();
	if (!Target)
	{
		return;
	}

	if (bAlignYawToFollowTargetOnRebuild)
	{
		const FRotator TargetRotation = Target->GetActorRotation();
		SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
	}

	const FVector TargetLocation = Target->GetActorLocation();
	float GroundZ = TargetLocation.Z;

	if (const ACharacter* Character = Cast<ACharacter>(Target))
	{
		if (const UCapsuleComponent* CapsuleComponent = Character->GetCapsuleComponent())
		{
			GroundZ = TargetLocation.Z - CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
	}
	else
	{
		FVector BoundsOrigin;
		FVector BoundsExtent;
		Target->GetActorBounds(false, BoundsOrigin, BoundsExtent);
		GroundZ = BoundsOrigin.Z - BoundsExtent.Z;
	}

	CourseLocalOrigin = GetActorTransform().InverseTransformPosition(FVector(TargetLocation.X, TargetLocation.Y, GroundZ));
}

AActor* ARunnerEndlessCourse::GetFollowTarget() const
{
	if (IsValid(FollowActor))
	{
		return FollowActor;
	}

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			return PlayerController->GetPawn();
		}
	}

	return nullptr;
}

AActor* ARunnerEndlessCourse::SpawnFromClassOrPrefab(TSubclassOf<AActor> ActorClass, AActor* ActorPrefab, const FVector& WorldLocation, const FRotator& WorldRotation)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	TSubclassOf<AActor> ClassToSpawn = ActorClass;
	if (ActorPrefab)
	{
		ClassToSpawn = ActorPrefab->GetClass();
	}

	if (!ClassToSpawn)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Template = ActorPrefab;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, WorldLocation, WorldRotation, SpawnParams);
	if (SpawnedActor && ActorPrefab)
	{
		SpawnedActor->SetActorHiddenInGame(false);
		SpawnedActor->SetActorEnableCollision(true);
	}

	return SpawnedActor;
}

TSubclassOf<AActor> ARunnerEndlessCourse::GetRandomFloorItemClass(const FRunnerRandomFloorItem& RandomFloorItem) const
{
	if (IsValid(RandomFloorItem.ItemPrefab))
	{
		return RandomFloorItem.ItemPrefab->GetClass();
	}

	if (IsValid(RandomFloorItem.ItemBlueprint) && RandomFloorItem.ItemBlueprint->GeneratedClass)
	{
		if (RandomFloorItem.ItemBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
		{
			return TSubclassOf<AActor>(RandomFloorItem.ItemBlueprint->GeneratedClass);
		}
	}

	return nullptr;
}

const FRunnerRandomFloorItem* ARunnerEndlessCourse::ChooseRandomFloorItem()
{
	float TotalWeight = 0.0f;
	for (const FRunnerRandomFloorItem& RandomFloorItem : RandomFloorItems)
	{
		if (GetRandomFloorItemClass(RandomFloorItem) && RandomFloorItem.SpawnWeight > 0.0f)
		{
			TotalWeight += RandomFloorItem.SpawnWeight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Pick = RandomStream.FRandRange(0.0f, TotalWeight);
	for (const FRunnerRandomFloorItem& RandomFloorItem : RandomFloorItems)
	{
		if (!GetRandomFloorItemClass(RandomFloorItem) || RandomFloorItem.SpawnWeight <= 0.0f)
		{
			continue;
		}

		Pick -= RandomFloorItem.SpawnWeight;
		if (Pick <= 0.0f)
		{
			return &RandomFloorItem;
		}
	}

	return nullptr;
}

bool ARunnerEndlessCourse::HasRandomFloorItems() const
{
	for (const FRunnerRandomFloorItem& RandomFloorItem : RandomFloorItems)
	{
		if (GetRandomFloorItemClass(RandomFloorItem) && RandomFloorItem.SpawnWeight > 0.0f)
		{
			return true;
		}
	}

	return false;
}

float ARunnerEndlessCourse::GetLocalFollowTargetX() const
{
	if (AActor* Target = GetFollowTarget())
	{
		return GetActorTransform().InverseTransformPosition(Target->GetActorLocation()).X - CourseLocalOrigin.X;
	}

	return 0.0f;
}

float ARunnerEndlessCourse::GetLaneOffset(int32 LaneIndex) const
{
	const int32 ClampedLaneCount = FMath::Max(LaneCount, 1);
	const int32 ClampedLaneIndex = FMath::Clamp(LaneIndex, 0, ClampedLaneCount - 1);
	const float CenteredLane = static_cast<float>(ClampedLaneIndex) - (static_cast<float>(ClampedLaneCount - 1) * 0.5f);
	return CenteredLane * LaneSpacing;
}
