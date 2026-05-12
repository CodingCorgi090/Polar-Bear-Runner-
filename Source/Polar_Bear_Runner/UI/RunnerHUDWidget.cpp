#include "UI/RunnerHUDWidget.h"

#include "Polar_Bear_Runner.h"
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

void URunnerHUDWidget::HideGameOver()
{
	bGameOverShown = false;
	BP_OnGameOverHidden();
}

float URunnerHUDWidget::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
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
	
	BP_OnScoreUpdated(NewScore);
}

// Updates the level on the UI
void URunnerHUDWidget::UpdateLevel(int32 const NewLevel)
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("CurrentLevel:  %d"), CurrentLevel);
	// First, update the CurrentScore property on the HUD class instance
	CurrentLevel = NewLevel;
	
	// Formats text to display in the HUD
	FText const LevelDisplayComplete = FText::Format(FText::FromString("Level {0}"), NewLevel);
	
	// Verify that the text block exists and update the HUD score
	if (LevelLabel)
	{
		LevelLabel->SetText(LevelDisplayComplete);
	}
	
}

// Updates the level progress on the UI
void URunnerHUDWidget::UpdateLevelProgress()
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("CurrentScore:  %d"), CurrentScore);
	
	float ProgressPercent = FMath::Clamp((CurrentScore % 10 / 10.0f), 0.0f, 1.0f);
	
	if (LevelProgress)
	{
		LevelProgress->SetPercent(ProgressPercent);
	}
}