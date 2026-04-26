// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerObstacle.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
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

	/** Visual mesh for the obstacle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* ObstacleMesh;

	/** Collision volume – resize this independently of the mesh for fine-tuned hit detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* HitBox;

public:

	ARunnerObstacle();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Attempts to damage the supplied actor if it is the runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Obstacle|Damage")
	bool TryDamageActor(AActor* OtherActor);

protected:

	virtual void BeginPlay() override;

	/** Bound to damage collision component overlaps. */
	UFUNCTION()
	void OnDamageOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                          bool bFromSweep, const FHitResult& SweepResult);

	void ConfigureDamageCollision();
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
};


