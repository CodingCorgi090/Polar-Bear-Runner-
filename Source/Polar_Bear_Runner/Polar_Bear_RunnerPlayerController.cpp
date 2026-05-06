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

void APolar_Bear_RunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("PlayerController BeginPlay called."));

	if (IsLocalPlayerController() && RunnerHUDWidgetClass)
	{
		RunnerHUDWidget = CreateWidget<URunnerHUDWidget>(this, RunnerHUDWidgetClass);
		if (RunnerHUDWidget)
		{
			RunnerHUDWidget->AddToPlayerScreen(1);
			RunnerHUDWidget->UpdateScore(0);
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

void APolar_Bear_RunnerPlayerController::HandleRunnerDied(ERunnerDamageType DamageType, AActor* DamageCauser)
{
	(void)DamageCauser;

	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

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
	}

	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);
	}

	// Instead of restarting level, respawn the player
	if (APolar_Bear_RunnerCharacter* RunnerCharacter = Cast<APolar_Bear_RunnerCharacter>(GetPawn()))
	{
		(void)RunnerCharacter;
		RespawnRunnerAfterDeath();
	}
}

void APolar_Bear_RunnerPlayerController::RespawnRunnerAfterDeath()
{
	UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("Respawn timer fired"));

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
	}

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (RunnerHUDWidget)
	{
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

	HandleRunnerHealthChanged(RunnerCharacter->GetCurrentHealth(), RunnerCharacter->GetMaxHealthValue());
}

void APolar_Bear_RunnerPlayerController::UnbindFromRunnerCharacter(APolar_Bear_RunnerCharacter* RunnerCharacter)
{
	if (!RunnerCharacter)
	{
		return;
	}

	RunnerCharacter->OnRunnerHealthChanged.RemoveDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerHealthChanged);
	RunnerCharacter->OnRunnerDied.RemoveDynamic(this, &APolar_Bear_RunnerPlayerController::HandleRunnerDied);
}

// Used to pass an updated score to the UI
void APolar_Bear_RunnerPlayerController::ReportScoreChange(int32 const Score)
{	
	//Passes the new score to the HUD widget instance if it exists
	if (RunnerHUDWidget) 
	{
		RunnerHUDWidget->UpdateScore(Score);
	}



}
