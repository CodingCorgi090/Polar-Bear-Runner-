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
 *    RequestDamageFromObstacle() on APolar_Bear_RunnerCharacter.
 *  - Set DamageOverride > 0 to use a custom damage value instead of
 *    the character's default ObstacleHitDamage.
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

	/** Attempts to damage the supplied actor if it is the runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Obstacle|Damage")
	bool TryDamageActor(AActor* OtherActor);

protected:

	virtual void BeginPlay() override;

	/** Bound to HitBox's OnComponentBeginOverlap */
	UFUNCTION()
	void OnHitBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                          bool bFromSweep, const FHitResult& SweepResult);

public:

	// ── Configuration ─────────────────────────────────────────────────────────

	/** If true, touching the hit box will automatically try to damage the runner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Damage")
	bool bDamagePlayerOnOverlap = true;

	/** Damage to deal. When <= 0 the character's default ObstacleHitDamage is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Damage",
	          meta=(ClampMin="0.0", UIMin="0.0", ToolTip="0 uses the runner's ObstacleHitDamage value. Any value above 0 overrides it for this obstacle."))
	float DamageOverride = 0.0f;

	/** If true, the obstacle destroys itself after hitting the player once. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle|Damage")
	bool bDestroyOnHit = false;

	/** Toggle to temporarily disable this obstacle without removing it from the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Obstacle")
	bool bIsActive = true;

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


