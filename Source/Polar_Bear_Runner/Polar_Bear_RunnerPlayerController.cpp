// Copyright Epic Games, Inc. All Rights Reserved.


#include "Polar_Bear_RunnerPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Polar_Bear_Runner.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Widgets/Input/SVirtualJoystick.h"

void APolar_Bear_RunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();

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

