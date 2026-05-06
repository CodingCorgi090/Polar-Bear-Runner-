// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerKeyPickup.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Polar_Bear_Runner.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Polar_Bear_RunnerPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ARunnerKeyPickup::ARunnerKeyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	SetRootComponent(KeyMesh);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KeyMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultCubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DefaultCubeMesh.Succeeded())
	{
		KeyMesh->SetStaticMesh(DefaultCubeMesh.Object);
	}

	CollectTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CollectTrigger"));
	CollectTrigger->SetupAttachment(RootComponent);
	CollectTrigger->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollectTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollectTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	CollectTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollectTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollectTrigger->SetGenerateOverlapEvents(true);
}

void ARunnerKeyPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (KeyMeshAsset)
	{
		KeyMesh->SetStaticMesh(KeyMeshAsset);
	}
}

void ARunnerKeyPickup::BeginPlay()
{
	Super::BeginPlay();

	CollectTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerKeyPickup::OnCollectTriggerOverlapBegin);
	CollectTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARunnerKeyPickup::OnCollectTriggerOverlapBegin);
}

bool ARunnerKeyPickup::TryCollect(AActor* OtherActor)
{
	if (!bCanBeCollected || bCollected)
	{
		return false;
	}

	APolar_Bear_RunnerCharacter* Runner = Cast<APolar_Bear_RunnerCharacter>(OtherActor);
	if (!Runner)
	{
		return false;
	}

	bCollected = true;

	if (bDisableCollisionOnCollect)
	{
		SetActorEnableCollision(false);
		CollectTrigger->SetGenerateOverlapEvents(false);
	}

	if (bHideOnCollect)
	{
		SetActorHiddenInGame(true);
	}

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("RunnerKeyPickup '%s' collected by '%s' for value %d."),
		*GetNameSafe(this), *GetNameSafe(Runner), KeyValue);

	//OnKeyCollected.Broadcast(Runner, KeyValue, this);
	//BP_OnKeyCollected(Runner, KeyValue, this);
	
	//Gets the controller
	APolar_Bear_RunnerPlayerController* Controller = Cast<APolar_Bear_RunnerPlayerController>(Runner->GetController());
	
	//If there's a collision with a key, add to the score
	//Returns bool
	if (Runner->AddScore(1))
	{
		//If score was updated, then get the new score for the UI
		int32 const NewScore = Runner->GetScore();
		
		//The controller handles the HUD
		//Report that the score has been changed if the controller exists
		if (Controller)
		{
			Controller->ReportScoreChange(NewScore);
		}
	}

	if (bDestroyOnCollect)
	{
		Destroy();
	}

	return true;
}

void ARunnerKeyPickup::OnCollectTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	TryCollect(OtherActor);
}

