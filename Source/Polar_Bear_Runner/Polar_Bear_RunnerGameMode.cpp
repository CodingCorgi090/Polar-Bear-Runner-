// Copyright Epic Games, Inc. All Rights Reserved.

#include "Polar_Bear_RunnerGameMode.h"

#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

APolar_Bear_RunnerGameMode::APolar_Bear_RunnerGameMode()
{
	// stub
}

void APolar_Bear_RunnerGameMode::TriggerGameOverAndRestart(AController* DeadController)
{
	(void)DeadController;

	if (bRestartQueued)
	{
		return;
	}

	bRestartQueued = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RestartTimerHandle, this, &APolar_Bear_RunnerGameMode::RestartRunLevel, RestartDelaySeconds, false);
	}
}

void APolar_Bear_RunnerGameMode::RestartRunLevel()
{
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);


		FName TargetLevel = RestartLevelName;
		if (bRestartCurrentLevel || TargetLevel.IsNone())
		{
			TargetLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
		}

		if (!TargetLevel.IsNone())
		{
			UGameplayStatics::OpenLevel(this, TargetLevel);
		}
	}

	bRestartQueued = false;
}

