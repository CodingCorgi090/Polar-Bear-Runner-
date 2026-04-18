// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Polar_Bear_RunnerGameMode.generated.h"

struct FTimerHandle;

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class APolar_Bear_RunnerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	APolar_Bear_RunnerGameMode();

	/** Starts game-over flow and queues level restart. */
	UFUNCTION(BlueprintCallable, Category="Runner|GameOver")
	void TriggerGameOverAndRestart(AController* DeadController);

protected:
	/** Time to wait before loading the level from the beginning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|GameOver", meta=(ClampMin="0.0", UIMin="0.0"))
	float RestartDelaySeconds = 2.0f;

	/** If true, restart the current level. If false, use RestartLevelName when set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|GameOver")
	bool bRestartCurrentLevel = true;

	/** Optional explicit level name to load when bRestartCurrentLevel is false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|GameOver")
	FName RestartLevelName;

private:
	void RestartRunLevel();

	FTimerHandle RestartTimerHandle;
	bool bRestartQueued = false;
};



