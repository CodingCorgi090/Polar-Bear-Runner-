// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Polar_Bear_RunnerPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class AActor;
class APolar_Bear_RunnerCharacter;
class URunnerHUDWidget;
enum class ERunnerDamageType : uint8;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class APolar_Bear_RunnerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Widget class used for health/game-over UI. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|UI")
	TSubclassOf<URunnerHUDWidget> RunnerHUDWidgetClass;

	/** Spawned runner HUD widget instance. */
	UPROPERTY(BlueprintReadOnly, Category="Runner|UI")
	TObjectPtr<URunnerHUDWidget> RunnerHUDWidget;

	/** Time dilation applied on death to accent game over before restart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|UI", meta=(ClampMin="0.05", ClampMax="1.0", UIMin="0.05", UIMax="1.0"))
	float DeathTimeDilation = 0.2f;

	/** Real-time delay before reviving the runner after death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Respawn", meta=(ClampMin="0.0", UIMin="0.0", Units="s"))
	float RespawnDelaySeconds = 1.0f;

	UFUNCTION()
	void HandleRunnerHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void HandleRunnerDied(ERunnerDamageType DamageType, AActor* DamageCauser);

	void BindToRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter);

	void UnbindFromRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter);

	void RespawnRunnerAfterDeath();

	FTimerHandle RespawnTimerHandle;

public:
	/** Routes missed-key damage to the currently possessed runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportMissedKeyDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Routes obstacle-hit damage to the currently possessed runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportObstacleDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

};
