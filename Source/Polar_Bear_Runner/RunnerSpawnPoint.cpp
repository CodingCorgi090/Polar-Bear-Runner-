// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerSpawnPoint.h"

// Sets default values
ARunnerSpawnPoint::ARunnerSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ARunnerSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARunnerSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
