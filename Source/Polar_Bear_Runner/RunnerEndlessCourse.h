// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerEndlessCourse.generated.h"

class UBoxComponent;
class UBlueprint;
class USceneComponent;
class UStaticMeshComponent;
class ARunnerFloorTile;

USTRUCT(BlueprintType)
struct FRunnerRandomFloorItem
{
	GENERATED_BODY()

	/** Blueprint asset to spawn onto the generated floor. Use this in Class Defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning")
	TObjectPtr<UBlueprint> ItemBlueprint = nullptr;

	/** Optional placed actor template to clone. Use this on a placed RunnerEndlessCourse instance. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Runner|Endless Course|Spawning")
	TObjectPtr<AActor> ItemPrefab = nullptr;

	/** Relative chance to pick this item when Random Floor Items are used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0.0", UIMin="0.0"))
	float SpawnWeight = 1.0f;

	/** Extra height above the floor surface after the actor's bottom is placed on the floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(Units="cm"))
	float ZOffset = 0.0f;
};

USTRUCT()
struct FRunnerCourseTile
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> FloorActor = nullptr;

	UPROPERTY()
	TObjectPtr<UBoxComponent> FloorCollision = nullptr;

	UPROPERTY()
	TObjectPtr<UBoxComponent> LeftEdgeBlocker = nullptr;

	UPROPERTY()
	TObjectPtr<UBoxComponent> RightEdgeBlocker = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;

	float CenterX = 0.0f;
	int32 TileNumber = 0;
};

/**
 * Runtime endless-runner course generator.
 *
 * Place one in the level, assign your Key and Obstacle Blueprints, and it will:
 * - keep floor tiles recycled in front of the player,
 * - randomly spawn keys and obstacles in lanes,
 * - create invisible side blockers so the player cannot leave the floor edges.
 */
UCLASS(Blueprintable, BlueprintType)
class POLAR_BEAR_RUNNER_API ARunnerEndlessCourse : public AActor
{
	GENERATED_BODY()

public:
	ARunnerEndlessCourse();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Clears and rebuilds the runtime course from the current settings. */
	UFUNCTION(BlueprintCallable, Category="Runner|Endless Course")
	void RebuildCourse();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Transient)
	TObjectPtr<AActor> FollowActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Runtime")
	bool bSnapToFollowTargetOnRebuild = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Runtime")
	bool bAlignYawToFollowTargetOnRebuild = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Floor", meta=(ClampMin="3", UIMin="3"))
	int32 TileCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Edges", meta=(ClampMin="50.0", UIMin="50.0", Units="cm"))
	float EdgeWallHeight = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Edges", meta=(ClampMin="1.0", UIMin="1.0", Units="cm"))
	float EdgeWallThickness = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Floor", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float RecycleBehindDistance = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Endless Course|Floor")
	float TileLength = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Endless Course|Floor")
	float TileWidth = 1600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Endless Course|Floor")
	float FloorThickness = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Floor")
	TSubclassOf<ARunnerFloorTile> FloorTileClass;

	/** Optional placed template actor to clone for floor tiles. Leave blank to use Floor Tile Class. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Runner|Endless Course|Floor")
	TObjectPtr<ARunnerFloorTile> FloorTilePrefab = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning")
	TArray<FRunnerRandomFloorItem> RandomFloorItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float RandomFloorItemSpawnChance = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Fallback Classes")
	TSubclassOf<AActor> KeyPickupClass;

	/** Optional placed template actor to clone for keys. Leave blank to use Key Pickup Class. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Runner|Endless Course|Fallback Classes")
	TObjectPtr<AActor> KeyPickupPrefab = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Fallback Classes")
	TSubclassOf<AActor> ObstacleClass;

	/** Optional placed template actor to clone for obstacles. Leave blank to use Obstacle Class. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Runner|Endless Course|Fallback Classes")
	TObjectPtr<AActor> ObstaclePrefab = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="1", UIMin="1"))
	int32 SpawnRowsPerTile = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="1", UIMin="1"))
	int32 LaneCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="1.0", UIMin="1.0", Units="cm"))
	float LaneSpacing = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float KeySpawnChance = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float ObstacleSpawnChance = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0", UIMin="0"))
	int32 SafeStartTiles = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
	float MinSpawnDistanceAhead = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(Units="cm"))
	float KeyZOffset = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Spawning", meta=(Units="cm"))
	float ObstacleZOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Random")
	bool bUseRandomSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Endless Course|Random", meta=(EditCondition="bUseRandomSeed"))
	int32 RandomSeed = 1337;

private:
	UPROPERTY(Transient)
	TArray<FRunnerCourseTile> Tiles;

	FRandomStream RandomStream;
	FVector CourseLocalOrigin = FVector::ZeroVector;
	int32 NextTileNumber = 0;

	void CreateInitialTiles();
	void CreateTile(int32 TileNumber);
	void ConfigureTile(FRunnerCourseTile& Tile, int32 TileNumber);
	void ClearTileContents(FRunnerCourseTile& Tile);
	void ClearCourse();
	void RecycleTilesIfNeeded();
	void SpawnContentsForTile(FRunnerCourseTile& Tile);
	void SpawnActorOnTile(FRunnerCourseTile& Tile, TSubclassOf<AActor> ActorClass, AActor* ActorPrefab, float LocalX, int32 LaneIndex, float ZOffset);
	void EnsureRuntimeDefaults();
	void ApplyFloorDimensionsFromClass();
	void ConfigureFloorActor(FRunnerCourseTile& Tile);
	void HideAuthoredStaticMeshComponents();
	void HidePrefabActors();
	void SnapCourseToFollowTarget();

	AActor* GetFollowTarget() const;
	AActor* SpawnFromClassOrPrefab(TSubclassOf<AActor> ActorClass, AActor* ActorPrefab, const FVector& WorldLocation, const FRotator& WorldRotation);
	TSubclassOf<AActor> GetRandomFloorItemClass(const FRunnerRandomFloorItem& RandomFloorItem) const;
	const FRunnerRandomFloorItem* ChooseRandomFloorItem();
	bool HasRandomFloorItems() const;
	float GetLocalFollowTargetX() const;
	float GetLaneOffset(int32 LaneIndex) const;
};
