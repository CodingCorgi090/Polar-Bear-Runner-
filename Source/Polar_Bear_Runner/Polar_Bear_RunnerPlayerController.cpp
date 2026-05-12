// Copyright Epic Games, Inc. All Rights Reserved.


#include "Polar_Bear_RunnerPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Polar_Bear_Runner.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Polar_Bear_RunnerGameMode.h"
#include "UI/RunnerHUDWidget.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FileManager/FileHandlers.h"

void APolar_Bear_RunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("PlayerController BeginPlay called."));

	if (IsLocalPlayerController())
	{
		TSubclassOf<URunnerHUDWidget> WidgetClassToCreate = RunnerHUDWidgetClass;
		if (!WidgetClassToCreate)
		{
			WidgetClassToCreate = URunnerHUDWidget::StaticClass();
		}

		RunnerHUDWidget = CreateWidget<URunnerHUDWidget>(this, WidgetClassToCreate);
		if (RunnerHUDWidget)
		{
			
			RunnerHUDWidget->AddToPlayerScreen(1);
			RunnerHUDWidget->UpdateScore(0);
			RunnerHUDWidget->UpdateLevelProgress();
			RunnerHUDWidget->UpdateLevel(0);
			
			// Creates an instance of the file handler
			UFile_Handler* FileHandler = NewObject<UFile_Handler>(this);
			// Gets the new list of scores
			TArray<FString> MyScores = FileHandler->GetScores();
			// Logs each score
			for (int32 index = 0; index < MyScores.Num(); ++index)
			{
				UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Here's score %d: %s"), index, *MyScores[index]);
			}
		}
		
		if (APolar_Bear_RunnerCharacter* NewRunner = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
		{
			NewRunner->ResetScore();
			NewRunner->ResetPlayerLevel();
			NewRunner->ResetRunnerAccel();
		}
		
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogPolar_Bear_Runner, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("PlayerController binding to runner character."));
	BindToRunnerCharacter(Cast<APolar_Bear_RunnerCharacter>(GetPawn()));
}

void APolar_Bear_RunnerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("PlayerController OnPossess called. Pawn=%s"), *GetNameSafe(InPawn));
	BindToRunnerCharacter(Cast<APolar_Bear_RunnerCharacter>(InPawn));
}

void APolar_Bear_RunnerPlayerController::OnUnPossess()
{
	ClearRespawnCountdown();
	UnbindFromRunnerCharacter(Cast<APolar_Bear_RunnerCharacter>(GetPawn()));
	Super::OnUnPossess();
}

void APolar_Bear_RunnerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool APolar_Bear_RunnerPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

bool APolar_Bear_RunnerPlayerController::ReportMissedKeyDamage(float DamageOverride, AActor* DamageCauser)
{
	if (APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
	{
		return RunnerCharacter->RequestDamageFromMissedKey(DamageOverride, DamageCauser);
	}

	return false;
}

bool APolar_Bear_RunnerPlayerController::ReportObstacleDamage(float DamageOverride, AActor* DamageCauser)
{
	if (APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
	{
		return RunnerCharacter->RequestDamageFromObstacle(DamageOverride, DamageCauser);
	}

	return false;
}

void APolar_Bear_RunnerPlayerController::HandleRunnerHealthChanged(float NewHealth, float MaxHealth)
{
	if (RunnerHUDWidget)
	{
		RunnerHUDWidget->UpdateHealth(NewHealth, MaxHealth);
	}
}

void APolar_Bear_RunnerPlayerController::HandleRunnerScoreChanged(int32 NewScore, int32 Delta)
{
	(void)Delta;
	CurrentScore = NewScore;
	ReportScoreChange(NewScore);
}

void APolar_Bear_RunnerPlayerController::HandleRunnerDied(ERunnerDamageType DamageType, AActor* DamageCauser)
{
	(void)DamageCauser;

	ClearRespawnCountdown();
	bWaitingForContinueChoice = true;

	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (DamageType == ERunnerDamageType::ObstacleHit)
	{
		if (APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
		{
			RunnerCharacter->SetActorEnableCollision(false);
			RunnerCharacter->SetActorHiddenInGame(true);
		}
	}

	if (RunnerHUDWidget)
	{
		if (const APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
		{
			RunnerHUDWidget->ShowGameOver(RunnerCharacter->GetCurrentHealth(), RunnerCharacter->GetMaxHealthValue());
		}

		RunnerHUDWidget->ShowContinuePrompt();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);
	}
}

void APolar_Bear_RunnerPlayerController::ContinueAfterDeath()
{
	if (bRespawnCountdownActive)
	{
		return;
	}

	if (!bWaitingForContinueChoice)
	{
		UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("ContinueAfterDeath called while not waiting for a continue choice."));
		return;
	}

	bWaitingForContinueChoice = false;
	StartRespawnCountdown();
}

void APolar_Bear_RunnerPlayerController::QuitAfterDeath()
{
	ClearRespawnCountdown();
	bWaitingForContinueChoice = false;

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Player chose not to continue. Quitting game."));
	UFile_Handler* FileHandler = NewObject<UFile_Handler>(this);
	// Saves the score change value
	FileHandler->SaveScores(CurrentScore);
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void APolar_Bear_RunnerPlayerController::StartRespawnCountdown()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		RespawnRunnerAfterDeath();
		return;
	}

	bRespawnCountdownActive = true;
	RespawnCountdownRemaining = FMath::Max(RespawnCountdownSeconds, 1);

	if (RunnerHUDWidget)
	{
		RunnerHUDWidget->ShowRespawnCountdown(RespawnCountdownRemaining);
	}

	World->GetTimerManager().SetTimer(
		RespawnCountdownTimerHandle,
		this,
		&APolar_Bear_RunnerPlayerController::AdvanceRespawnCountdown,
		1.0f,
		true);
}

void APolar_Bear_RunnerPlayerController::AdvanceRespawnCountdown()
{
	if (!bRespawnCountdownActive)
	{
		return;
	}

	--RespawnCountdownRemaining;

	if (RespawnCountdownRemaining <= 0)
	{
		ClearRespawnCountdown();
		RespawnRunnerAfterDeath();
		return;
	}

	if (RunnerHUDWidget)
	{
		RunnerHUDWidget->ShowRespawnCountdown(RespawnCountdownRemaining);
	}
}

void APolar_Bear_RunnerPlayerController::ClearRespawnCountdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnCountdownTimerHandle);
	}

	bRespawnCountdownActive = false;
	RespawnCountdownRemaining = 0;
}

void APolar_Bear_RunnerPlayerController::RespawnRunnerAfterDeath()
{
	UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("Respawn timer fired"));
	ClearRespawnCountdown();

	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);
	}

	APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn());
	if (!RunnerCharacter)
	{
		UE_LOG(LogPolar_Bear_Runner, Error, TEXT("Could not get character in respawn callback!"));
	}
	else
	{
		RunnerCharacter->RespawnPlayer();
		ReportScoreChange(CurrentScore);
		
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (RunnerHUDWidget)
	{
		RunnerHUDWidget->HideRespawnCountdown();
		RunnerHUDWidget->HideGameOver();
	}

	UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("Respawn complete!"));
}

void APolar_Bear_RunnerPlayerController::BindToRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter)
{
	if (!RunnerCharacter)
	{
		UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("BindToRunnerCharacter called with null RunnerCharacter"));
		return;
	}

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Binding to RunnerCharacter %s"), *GetNameSafe(RunnerCharacter));

	UnbindFromRunnerCharacter(RunnerCharacter);
	RunnerCharacter->OnRunnerHealthChanged.AddDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerHealthChanged);
	RunnerCharacter->OnRunnerDied.AddDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerDied);
	RunnerCharacter->OnRunnerScoreChanged.AddDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerScoreChanged);
	
	HandleRunnerHealthChanged(RunnerCharacter->GetCurrentHealth(), RunnerCharacter->GetMaxHealthValue());
	HandleRunnerScoreChanged(RunnerCharacter->GetScore(), 0);
}

void APolar_Bear_RunnerPlayerController::UnbindFromRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter)
{
	if (!RunnerCharacter)
	{
		return;
	}

	RunnerCharacter->OnRunnerHealthChanged.RemoveDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerHealthChanged);
	RunnerCharacter->OnRunnerDied.RemoveDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerDied);
	RunnerCharacter->OnRunnerScoreChanged.RemoveDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerScoreChanged);
}

// Used to pass an updated score to the UI
void APolar_Bear_RunnerPlayerController::ReportScoreChange(int32 const Score)
{	
	if (RunnerHUDWidget) 
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("HUD Exists!"));
		RunnerHUDWidget->UpdateScore(Score);
		RunnerHUDWidget->UpdateLevelProgress();
	}
}

void APolar_Bear_RunnerPlayerController::ReportLevelUpdate(int32 const NewLevel)
{
	if (RunnerHUDWidget)
	{
		RunnerHUDWidget->UpdateLevel(NewLevel);
	}
	
}
