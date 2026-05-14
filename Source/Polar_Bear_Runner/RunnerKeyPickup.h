// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunnerKeyPickup.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UStaticMesh;
class UMaterialInterface;
class UTextRenderComponent;
class USceneComponent;
class APolar_Bear_RunnerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRunnerKeyCollectedSignature, APolar_Bear_RunnerCharacter*, RunnerCharacter, int32, KeyValue, AActor*, KeyActor);

/**
 * Placeable musical key pickup.
 *
 * - On overlap with the runner character, the key is marked collected and can hide itself.
 * - Exposes key value/events so score systems can be added later without changing pickup collision code.
 */
UCLASS(Blueprintable, BlueprintType)
class POLAR_BEAR_RUNNER_API ARunnerKeyPickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* KeyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UBoxComponent* CollectTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UTextRenderComponent* KeyLabel;

public:
	ARunnerKeyPickup();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Returns true if the supplied actor successfully collected this key. */
	UFUNCTION(BlueprintCallable, Category="Runner|Key")
	bool TryCollect(AActor* OtherActor);

	/** Reapplies the hard-coded flat key pickup shape. */
	UFUNCTION(BlueprintCallable, Category="Runner|Key|Visual")
	void RefreshKeyShape();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCollectTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ApplyVisualSettings();
	void ApplyKeyMaterial(UMaterialInterface* PreferredMaterial);
	void ConfigureLabel();
	void UpdateLabelFacingPlayer();
	void UpdateCollectTriggerFromVisualSize();

public:
	/** Score value for this key. Not consumed yet by game systems, but exposed for future score integration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key", meta=(ClampMin="0", UIMin="0"))
	int32 KeyValue = 2;

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

	/** Optional material override. If unset, the Blueprint KeyMesh material is preserved when C++ reshapes the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Visual")
	TObjectPtr<UMaterialInterface> KeyMaterial = nullptr;

	/** If true, the visual mesh is scaled into a flat floor pickup shape. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Visual")
	bool bMakeKeyCubeLike = true;

	/** Final visual size used when bMakeKeyCubeLike is enabled. Kept thin so purple keys lie flat on the floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Visual",
	          meta=(EditCondition="bMakeKeyCubeLike", ClampMin="1.0", UIMin="1.0"))
	FVector KeyVisualSize = FVector(170.0f, 170.0f, 8.0f);

	/** Padding added around the cube-like key for collection overlap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Collision",
	          meta=(ClampMin="0.0", UIMin="0.0"))
	FVector CollectTriggerPadding = FVector(12.0f, 12.0f, 80.0f);

	/** Text shown on the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label")
	FText KeyLabelText;

	/** If true, the label rotates so it faces the player/camera side while staying upright. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label")
	bool bLabelFacesPlayer = false;

	/** Local position of the label. Negative X places it on the side facing the runner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label")
	FVector LabelRelativeLocation = FVector(0.0f, 0.0f, 7.0f);

	/** Height of the rendered label in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label",
	          meta=(ClampMin="1.0", UIMin="1.0"))
	float LabelWorldSize = 32.0f;

	/** Color of the rendered label. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label")
	FColor LabelColor = FColor::Black;

	/** Hide this if the mesh/material already has its own readable letter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Key|Label")
	bool bShowLabel = true;

	/** Fired when a key is collected (future score systems can bind here). */
	UPROPERTY(BlueprintAssignable, Category="Runner|Key|Events")
	FRunnerKeyCollectedSignature OnKeyCollected;

	/** BP hook for VFX/SFX/UI updates when key is collected. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Key|Events")
	void BP_OnKeyCollected(APolar_Bear_RunnerCharacter* RunnerCharacter, int32 CollectedValue, AActor* KeyActor);

	FORCEINLINE bool IsCollected() const { return bCollected; }
	FORCEINLINE UStaticMeshComponent* GetKeyMesh() const { return KeyMesh; }
	FORCEINLINE UBoxComponent* GetCollectTrigger() const { return CollectTrigger; }
	FORCEINLINE UTextRenderComponent* GetKeyLabel() const { return KeyLabel; }
	FORCEINLINE USceneComponent* GetSceneRoot() const { return SceneRoot; }

private:
	UPROPERTY(VisibleAnywhere, Category="Runner|Key")
	bool bCollected = false;
};

