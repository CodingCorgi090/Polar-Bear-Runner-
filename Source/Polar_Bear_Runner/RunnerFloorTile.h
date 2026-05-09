// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerFloorTile.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Blueprintable straight floor tile for the endless course.
 *
 * The actor origin is the top-center of the floor so pickups and obstacles can
 * be placed directly on the surface.
 */
UCLASS(Blueprintable, BlueprintType)
class POLAR_BEAR_RUNNER_API ARunnerFloorTile : public AActor
{
	GENERATED_BODY()

public:
	ARunnerFloorTile();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category="Runner|Floor Tile")
	float GetTileLength() const { return TileLength; }

	UFUNCTION(BlueprintPure, Category="Runner|Floor Tile")
	float GetTileWidth() const { return TileWidth; }

	UFUNCTION(BlueprintPure, Category="Runner|Floor Tile")
	float GetTileThickness() const { return TileThickness; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> FloorMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Floor Tile")
	TObjectPtr<UStaticMesh> FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Floor Tile", meta=(ClampMin="100.0", UIMin="100.0", Units="cm"))
	float TileLength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Floor Tile", meta=(ClampMin="100.0", UIMin="100.0", Units="cm"))
	float TileWidth = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Floor Tile", meta=(ClampMin="1.0", UIMin="1.0", Units="cm"))
	float TileThickness = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Floor Tile")
	bool bAutoScaleMeshToTile = true;

private:
	void ConfigureFloorMesh();
};
