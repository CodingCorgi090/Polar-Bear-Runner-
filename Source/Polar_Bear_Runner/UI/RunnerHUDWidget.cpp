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

void URunnerHUDWidget::HideGameOver()
{
	bGameOverShown = false;
	BP_OnGameOverHidden();
}

float URunnerHUDWidget::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}
