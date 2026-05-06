#include "UI/RunnerHUDWidget.h"
#include "Math/UnrealMathUtility.h"

void URunnerHUDWidget::UpdateHealth(float NewHealth, float InMaxHealth)
{
	MaxHealth = FMath::Max(InMaxHealth, 1.0f);
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	BP_OnHealthUpdated(CurrentHealth, MaxHealth, GetHealthPercent());
}

void URunnerHUDWidget::ShowGameOver(float FinalHealth, float InMaxHealth)
{
	UpdateHealth(FinalHealth, InMaxHealth);
	bGameOverShown = true;
	BP_OnGameOverShown(CurrentHealth, MaxHealth);
}

// Updates the score on the UI
void URunnerHUDWidget::UpdateScore(int32 const NewScore)
{
	// First, update the CurrentScore property on the HUD class instance
	CurrentScore = NewScore;
	
	// Formats text to display in the HUD
	FText const ScoreDisplayComplete = FText::Format(FText::FromString("Your Score: {0}"), NewScore);
	
	// Verify that the text block exists and update the HUD score
	if (ScoreBlock)
	{
		ScoreBlock->SetText(ScoreDisplayComplete);
	}
	
	// Updates blueprint event... not currently used
	// BP_OnScoreUpdated(NewScore);
}


void URunnerHUDWidget::HideGameOver()
{
	bGameOverShown = false;
	BP_OnGameOverHidden();
}

float URunnerHUDWidget::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
