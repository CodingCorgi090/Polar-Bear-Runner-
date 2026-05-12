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

	/** Seconds shown after the player chooses to continue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Respawn", meta=(ClampMin="1", UIMin="1"))
	int32 RespawnCountdownSeconds = 3;

	FTimerHandle RespawnCountdownTimerHandle;
	int32 RespawnCountdownRemaining = 0;
	bool bWaitingForContinueChoice = false;
	bool bRespawnCountdownActive = false;

	UFUNCTION()
	void HandleRunnerHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void HandleRunnerDied(ERunnerDamageType DamageType, AActor* DamageCauser);

	UFUNCTION()
	void HandleRunnerScoreChanged(int32 NewScore, int32 Delta);

	void BindToRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter);

	void UnbindFromRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter);

	void StartRespawnCountdown();
	void AdvanceRespawnCountdown();
	void ClearRespawnCountdown();
	void RespawnRunnerAfterDeath();
	
	UPROPERTY(EditAnywhere, Category ="Runner|Score")
	int32 CurrentScore;

public:
	/** Routes missed-key damage to the currently possessed runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportMissedKeyDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Kills the currently possessed runner character for obstacle hits. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportObstacleDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);
	
	//Hands the new score to the HUD
	UFUNCTION(BlueprintCallable, Category = "Runner|Score")
	void ReportScoreChange(int32 const Score);
	
	//Hands the new score to the HUD
	UFUNCTION(BlueprintCallable, Category = "Runner|Score")
	void ReportLevelUpdate(int32 const NewLevel);

	/** Call this from the game-over Yes button. Starts the respawn countdown. */
	UFUNCTION(BlueprintCallable, Category="Runner|GameOver")
	void ContinueAfterDeath();

	/** Call this from the game-over No button. Closes the game. */
	UFUNCTION(BlueprintCallable, Category="Runner|GameOver")
	void QuitAfterDeath();

};
