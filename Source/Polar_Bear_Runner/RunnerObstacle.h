// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerObstacle.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTextRenderComponent;
class USceneComponent;
class APolar_Bear_RunnerCharacter;

/**
 *  A placeable obstacle that damages the player on contact.
 *
 *  - Drop it into any level and it will automatically call
 *    KillRunner() on APolar_Bear_RunnerCharacter.
 *  - Enable bDestroyOnHit to make single-use hazards.
 *  - Implement BP_OnPlayerHit / BP_OnPlayerHitBlocked in Blueprint
 *    for VFX, SFX, or any other feedback.
 */
UCLASS(Blueprintable, BlueprintType)
class ARunnerObstacle : public AActor
{
	GENERATED_BODY()

	/** Unscaled root so visual sizing does not distort child collision or label text. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* SceneRoot;

	/** Visual mesh for the obstacle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* ObstacleMesh;

	/** Collision volume – resize this independently of the mesh for fine-tuned hit detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* HitBox;

	/** Letter/icon drawn on the player-facing side of the obstacle. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UTextRenderComponent* ObstacleLabel;

public:

	ARunnerObstacle();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Attempts to damage the supplied actor if it is the runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Obstacle|Damage")
	bool TryDamageActor(AActor* OtherActor);

	/** Reapplies the hard-coded endless runner obstacle shape. */
	UFUNCTION(BlueprintCallable, Category="Runner|Obstacle|Visual")
	void RefreshObstacleShape();

protected:

	virtual void BeginPlay() override;

	/** Bound to damage collision component overlaps. */
	UFUNCTION()
	void OnDamageOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                          bool bFromSweep, const FHitResult& SweepResult);

	void ConfigureDamageCollision();
	void ApplyVisualSettings();
	void ConfigureLabel();
	void UpdateLabelFacingPlayer();
	void UpdateHitBoxFromMeshBounds();

public:

	// ── Configuration ─────────────────────────────────────────────────────────

	/** If true, touching the hit box will automatically try to damage the runner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Damage")
	bool bDamagePlayerOnOverlap = true;

	/** If true, the obstacle destroys itself after hitting the player once. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Damage")
	bool bDestroyOnHit = false;

	/** If true, the damage hit box is resized to match the assigned mesh bounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Collision")
	bool bAutoSizeHitBoxToMesh = true;

	/** Extra size added to the auto-fitted damage hit box. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Collision",
	          meta=(EditCondition="bAutoSizeHitBoxToMesh", ClampMin="0.0", UIMin="0.0"))
	FVector HitBoxPadding = FVector(10.0f, 10.0f, 10.0f);

	/** Toggle to temporarily disable this obstacle without removing it from the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle")
	bool bIsActive = true;

	/** Optional mesh override; leave null for no C++ default so the Blueprint can choose its own mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Visual")
	TObjectPtr<UStaticMesh> ObstacleMeshAsset = nullptr;

	/** If true, the visual mesh is scaled into a cube-like obstacle shape. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Visual")
	bool bMakeObstacleCubeLike = true;

	/** Final visual size used when bMakeObstacleCubeLike is enabled. The largest side is used for all axes so it stays square. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Visual",
	          meta=(EditCondition="bMakeObstacleCubeLike", ClampMin="1.0", UIMin="1.0"))
	FVector ObstacleVisualSize = FVector(150.0f, 150.0f, 150.0f);

	/** Text shown on the obstacle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label")
	FText ObstacleLabelText;

	/** If true, the label rotates so it faces the player/camera side while staying upright. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label")
	bool bLabelFacesPlayer = false;

	/** Local position of the label. Negative X places it on the side facing the runner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label")
	FVector LabelRelativeLocation = FVector(-77.0f, 0.0f, 0.0f);

	/** Height of the rendered label in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label",
	          meta=(ClampMin="1.0", UIMin="1.0"))
	float LabelWorldSize = 55.0f;

	/** Color of the rendered label. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label")
	FColor LabelColor = FColor(230, 80, 50);

	/** Hide this if the mesh/material already has its own readable letter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Label")
	bool bShowLabel = true;

	// ── Blueprint hooks ────────────────────────────────────────────────────────

	/** Called when the player was successfully hit. Use for VFX / SFX / screen shake. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Obstacle")
	void BP_OnPlayerHit(APolar_Bear_RunnerCharacter* HitCharacter);

	/**
	 * Called when the player overlapped but damage was blocked
	 * (e.g. damage cooldown active or player already dead).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Obstacle")
	void BP_OnPlayerHitBlocked(APolar_Bear_RunnerCharacter* HitCharacter);

	// ── Accessors ──────────────────────────────────────────────────────────────

	FORCEINLINE UStaticMeshComponent* GetObstacleMesh() const { return ObstacleMesh; }
	FORCEINLINE UBoxComponent*        GetHitBox()       const { return HitBox; }
	FORCEINLINE UTextRenderComponent* GetObstacleLabel() const { return ObstacleLabel; }
	FORCEINLINE USceneComponent*      GetSceneRoot()    const { return SceneRoot; }
};


