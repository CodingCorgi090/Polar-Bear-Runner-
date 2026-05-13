#include "UI/RunnerHUDWidget.h"

#include "Polar_Bear_Runner.h"
#include "Polar_Bear_RunnerPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Math/UnrealMathUtility.h"
#include "Styling/CoreStyle.h"

void URunnerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFallbackContinuePrompt();
}

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
	HideRespawnCountdown();
	SetFallbackContinuePromptVisible(false);
	BP_OnGameOverHidden();
}

void URunnerHUDWidget::ShowContinuePrompt()
{
	bGameOverShown = true;
	HideRespawnCountdown();
	EnsureFallbackContinuePrompt();
	if (FallbackPromptText)
	{
		FallbackPromptText->SetText(FText::FromString(TEXT("Continue?")));
	}
	if (FallbackButtonRow)
	{
		FallbackButtonRow->SetVisibility(ESlateVisibility::Visible);
	}
	SetFallbackContinuePromptVisible(true);
	BP_OnContinuePromptShown();
}

void URunnerHUDWidget::ShowRespawnCountdown(int32 SecondsRemaining)
{
	CurrentRespawnCountdownSeconds = FMath::Max(SecondsRemaining, 0);

	if (RespawnCountdownBlock)
	{
		const FText CountdownText = FText::Format(FText::FromString("Respawning in {0}"), CurrentRespawnCountdownSeconds);
		RespawnCountdownBlock->SetText(CountdownText);
		RespawnCountdownBlock->SetVisibility(ESlateVisibility::Visible);
	}

	EnsureFallbackContinuePrompt();
	if (FallbackPromptText)
	{
		FallbackPromptText->SetText(FText::FromString(TEXT("Respawning")));
	}
	if (FallbackCountdownBlock)
	{
		const FText CountdownText = FText::Format(FText::FromString("Respawning in {0}"), CurrentRespawnCountdownSeconds);
		FallbackCountdownBlock->SetText(CountdownText);
	}
	if (FallbackButtonRow)
	{
		FallbackButtonRow->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetFallbackContinuePromptVisible(true);
	SetFallbackCountdownVisible(true);

	BP_OnRespawnCountdownUpdated(CurrentRespawnCountdownSeconds);
}

void URunnerHUDWidget::HideRespawnCountdown()
{
	CurrentRespawnCountdownSeconds = -1;

	if (RespawnCountdownBlock)
	{
		RespawnCountdownBlock->SetText(FText::GetEmpty());
		RespawnCountdownBlock->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetFallbackCountdownVisible(false);

	BP_OnRespawnCountdownHidden();
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
	
	constexpr int32 PointsPerLevel = 16;
	const float ProgressPercent = FMath::Clamp((CurrentScore % PointsPerLevel) / static_cast<float>(PointsPerLevel), 0.0f, 1.0f);
	
	if (LevelProgress)
	{
		LevelProgress->SetPercent(ProgressPercent);
	}
}

void URunnerHUDWidget::EnsureFallbackContinuePrompt()
{
	if (FallbackContinuePanel || WidgetTree == nullptr)
	{
		return;
	}

	UWidget* ExistingRoot = WidgetTree->RootWidget;
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(ExistingRoot);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RunnerHUDRootCanvas"));
		WidgetTree->RootWidget = RootCanvas;

		if (ExistingRoot)
		{
			UCanvasPanelSlot* ExistingRootSlot = RootCanvas->AddChildToCanvas(ExistingRoot);
			if (ExistingRootSlot)
			{
				ExistingRootSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				ExistingRootSlot->SetOffsets(FMargin(0.0f));
			}
		}
	}

	FallbackContinuePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FallbackContinuePanel"));
	FallbackContinuePanel->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.025f, 0.92f));
	FallbackContinuePanel->SetPadding(FMargin(28.0f));
	FallbackContinuePanel->SetVisibility(ESlateVisibility::Collapsed);

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(FallbackContinuePanel);
	if (PanelSlot)
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetSize(FVector2D(460.0f, 240.0f));
		PanelSlot->SetZOrder(1000);
	}

	UVerticalBox* PromptLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FallbackContinueLayout"));
	FallbackContinuePanel->SetContent(PromptLayout);

	FallbackPromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackContinueTitle"));
	FallbackPromptText->SetText(FText::FromString(TEXT("Continue?")));
	FallbackPromptText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FallbackPromptText->SetJustification(ETextJustify::Center);
	FallbackPromptText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 34));
	if (UVerticalBoxSlot* TitleSlot = PromptLayout->AddChildToVerticalBox(FallbackPromptText))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	UTextBlock* PromptBodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackContinueBody"));
	PromptBodyText->SetText(FText::FromString(TEXT("Do you want to keep running?")));
	PromptBodyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.86f, 0.86f, 0.9f, 1.0f)));
	PromptBodyText->SetJustification(ETextJustify::Center);
	PromptBodyText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 20));
	if (UVerticalBoxSlot* BodySlot = PromptLayout->AddChildToVerticalBox(PromptBodyText))
	{
		BodySlot->SetHorizontalAlignment(HAlign_Fill);
		BodySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
	}

	FallbackCountdownBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackRespawnCountdown"));
	FallbackCountdownBlock->SetText(FText::GetEmpty());
	FallbackCountdownBlock->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.72f, 0.35f, 1.0f)));
	FallbackCountdownBlock->SetJustification(ETextJustify::Center);
	FallbackCountdownBlock->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 26));
	FallbackCountdownBlock->SetVisibility(ESlateVisibility::Collapsed);
	if (UVerticalBoxSlot* CountdownSlot = PromptLayout->AddChildToVerticalBox(FallbackCountdownBlock))
	{
		CountdownSlot->SetHorizontalAlignment(HAlign_Fill);
		CountdownSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}

	FallbackButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FallbackContinueButtons"));
	if (UVerticalBoxSlot* ButtonRowSlot = PromptLayout->AddChildToVerticalBox(FallbackButtonRow))
	{
		ButtonRowSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UButton* YesButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FallbackContinueYesButton"));
	YesButton->OnClicked.AddDynamic(this, &URunnerHUDWidget::HandleFallbackContinueClicked);
	if (UHorizontalBoxSlot* YesSlot = FallbackButtonRow->AddChildToHorizontalBox(YesButton))
	{
		YesSlot->SetPadding(FMargin(0.0f, 0.0f, 16.0f, 0.0f));
	}

	UTextBlock* YesText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackContinueYesText"));
	YesText->SetText(FText::FromString(TEXT("Yes")));
	YesText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	YesText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 22));
	YesButton->AddChild(YesText);

	UButton* NoButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FallbackContinueNoButton"));
	NoButton->OnClicked.AddDynamic(this, &URunnerHUDWidget::HandleFallbackQuitClicked);
	FallbackButtonRow->AddChildToHorizontalBox(NoButton);

	UTextBlock* NoText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackContinueNoText"));
	NoText->SetText(FText::FromString(TEXT("No")));
	NoText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	NoText->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 22));
	NoButton->AddChild(NoText);
}

void URunnerHUDWidget::SetFallbackContinuePromptVisible(bool bVisible)
{
	if (FallbackContinuePanel)
	{
		FallbackContinuePanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void URunnerHUDWidget::SetFallbackCountdownVisible(bool bVisible)
{
	if (FallbackCountdownBlock)
	{
		FallbackCountdownBlock->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void URunnerHUDWidget::HandleFallbackContinueClicked()
{
	if (APolar_Bear_RunnerPlayerController* RunnerController = Cast<APolar_Bear_RunnerPlayerController>(GetOwningPlayer()))
	{
		RunnerController->ContinueAfterDeath();
	}
}

void URunnerHUDWidget::HandleFallbackQuitClicked()
{
	if (APolar_Bear_RunnerPlayerController* RunnerController = Cast<APolar_Bear_RunnerPlayerController>(GetOwningPlayer()))
	{
		RunnerController->QuitAfterDeath();
	}
}

// Updates the level on the UI
void URunnerHUDWidget::UpdateHighScore(int32 const HighScore)
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Current high Score:  %d"), HighScore);
	// First, update the CurrentScore property on the HUD class instance
	CurrentHighScore = HighScore;
	
	// Formats text to display in the HUD
	FText const HighScoreText = FText::Format(FText::FromString("Your High Score: {0}"), HighScore);
	
	// Verify that the text block exists and update the HUD score
	if (HighScoreLabel)
	{
		HighScoreLabel->SetText(HighScoreText);
	}
	
}
