#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
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
	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void UpdateHealth(float NewHealth, float InMaxHealth);

	UFUNCTION(BlueprintCallable, Category="Runner|UI")
	void ShowGameOver(float FinalHealth, float InMaxHealth);

	UFUNCTION(BlueprintPure, Category="Runner|UI")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category="Runner|UI")
	bool IsGameOverShown() const { return bGameOverShown; }

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnHealthUpdated(float NewHealth, float InMaxHealth, float HealthPercent);

	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnGameOverShown(float FinalHealth, float InMaxHealth);

	// Updates the player UI score
	UFUNCTION(BlueprintCallable, Category = "Runner|UI")
	void UpdateScore(int32 const NewScore);
	
	// Blueprint event to use to update UI score
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|UI")
	void BP_OnScoreUpdated(int32 const ScoreDisplay);
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	float MaxHealth = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|UI")
	bool bGameOverShown = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runner|UI")
	int CurrentScore = 0;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreBlock;
};

