// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Polar_Bear_RunnerGameMode.generated.h"

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
};



