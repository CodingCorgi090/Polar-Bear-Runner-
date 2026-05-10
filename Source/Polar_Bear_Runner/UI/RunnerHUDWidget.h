#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "RunnerHUDWidget.generated.h"


/**
 * Lightweight C++ base widget for runner health/game-over UI.
 * Implement the BP events in a Widget Blueprint for visuals.
 */
UCLASS(Blueprintable, BlueprintType)
class POLAR_BEAR_RUNNER_API URunnerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void UpdateHealth(float NewHealth, float InMaxHealth);

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void ShowGameOver(float FinalHealth, float InMaxHealth);

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void HideGameOver();

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void ShowContinuePrompt();

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void ShowRespawnCountdown(int32 SecondsRemaining);

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void HideRespawnCountdown();

	UFUNCTION(BlueprintPure, Category="Runner|UI")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category="Runner|UI")
	bool IsGameOverShown() const { return bGameOverShown; }

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnHealthUpdated(float NewHealth, float InMaxHealth, float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnGameOverShown(float FinalHealth, float InMaxHealth);

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnGameOverHidden();

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnContinuePromptShown();

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnRespawnCountdownUpdated(int32 SecondsRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnRespawnCountdownHidden();

	// Updates the player UI score
	UFUNCTION(BlueprintCallable, Category = "Runner|UI")
	void UpdateScore(int32 const NewScore);
	
	// Blueprint event to use to update UI score
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnScoreUpdated(int32 const ScoreDisplay);
	
	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void UpdateLevel(int32 const NewLevel);
	
	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void UpdateLevelProgress();

protected:
	void EnsureFallbackContinuePrompt();
	void SetFallbackContinuePromptVisible(bool bVisible);
	void SetFallbackCountdownVisible(bool bVisible);

	UFUNCTION()
	void HandleFallbackContinueClicked();

	UFUNCTION()
	void HandleFallbackQuitClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	float MaxHealth = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	bool bGameOverShown = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runner|UI")
	int32 CurrentScore = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runner|UI")
	int CurrentLevel = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	int32 CurrentRespawnCountdownSeconds = -1;
	
	UPROPERTY(BlueprintReadOnly, Category="Runner|UI", meta = (BindWidgetOptional))
	UTextBlock* ScoreBlock;

	UPROPERTY(BlueprintReadOnly, Category="Runner|UI", meta = (BindWidgetOptional))
	UTextBlock* RespawnCountdownBlock;

	UPROPERTY(Transient)
	TObjectPtr<class UBorder> FallbackContinuePanel;

	UPROPERTY(Transient)
	TObjectPtr<class UTextBlock> FallbackPromptText;

	UPROPERTY(Transient)
	TObjectPtr<class UTextBlock> FallbackCountdownBlock;

	UPROPERTY(Transient)
	TObjectPtr<class UHorizontalBox> FallbackButtonRow;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* LevelLabel;
	
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* LevelProgress;
};
