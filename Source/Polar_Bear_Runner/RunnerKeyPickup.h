// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerKeyPickup.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UStaticMesh;
class APolar_Bear_RunnerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRunnerKeyCollectedSignature, APolar_Bear_RunnerCharacter*, RunnerCharacter, int32, KeyValue, AActor*, KeyActor);

/**
 * Placeable musical key pickup.
 *
 * - On overlap with the runner character, the key is marked collected and can hide itself.
 * - Exposes key value/events so score systems can be added later without changing pickup collision code.
 */
UCLASS(Blueprintable, BlueprintType)
class ARunnerKeyPickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* KeyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* CollectTrigger;

public:
	ARunnerKeyPickup();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Returns true if the supplied actor successfully collected this key. */
	UFUNCTION(BlueprintCallable, Category="Runner|Key")
	bool TryCollect(AActor* OtherActor);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCollectTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	/** Score value for this key. Not consumed yet by game systems, but exposed for future score integration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key", meta=(ClampMin="0", UIMin="0"))
	int32 KeyValue = 1;

	/** If true, overlap with the runner will collect this key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key")
	bool bCanBeCollected = true;

	/** If true, key actor is hidden after being collected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key")
	bool bHideOnCollect = true;

	/** If true, all collision is disabled after collection to prevent repeat overlaps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key")
	bool bDisableCollisionOnCollect = true;

	/** Optional immediate destroy after collection (leave false while iterating). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key")
	bool bDestroyOnCollect = false;

	/** Optional mesh override; leave null to keep the mesh configured on the Blueprint KeyMesh component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Visual")
	UStaticMesh* KeyMeshAsset = nullptr;

	/** Fired when a key is collected (future score systems can bind here). */
	UPROPERTY(BlueprintAssignable, Category="Runner|Key|Events")
	FRunnerKeyCollectedSignature OnKeyCollected;

	/** BP hook for VFX/SFX/UI updates when key is collected. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Key|Events")
	void BP_OnKeyCollected(APolar_Bear_RunnerCharacter* RunnerCharacter, int32 CollectedValue, AActor* KeyActor);

	FORCEINLINE bool IsCollected() const { return bCollected; }
	FORCEINLINE UStaticMeshComponent* GetKeyMesh() const { return KeyMesh; }
	FORCEINLINE UBoxComponent* GetCollectTrigger() const { return CollectTrigger; }

private:
	UPROPERTY(VisibleAnywhere, Category="Runner|Key")
	bool bCollected = false;
};

