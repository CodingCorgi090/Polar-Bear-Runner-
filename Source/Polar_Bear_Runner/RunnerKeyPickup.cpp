// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerKeyPickup.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Polar_Bear_Runner.h"
#include "Polar_Bear_RunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ARunnerKeyPickup::ARunnerKeyPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(RootComponent);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KeyMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultFishMesh(TEXT("/Game/Models/Trout.Trout"));
	if (DefaultFishMesh.Succeeded())
	{
		KeyMeshAsset = DefaultFishMesh.Object;
		KeyMesh->SetStaticMesh(KeyMeshAsset);
	}

	CollectTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CollectTrigger"));
	CollectTrigger->SetupAttachment(RootComponent);
	CollectTrigger->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollectTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollectTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	CollectTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollectTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollectTrigger->SetGenerateOverlapEvents(true);

	KeyLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("KeyLabel"));
	KeyLabel->SetupAttachment(RootComponent);
	KeyLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KeyLabel->SetGenerateOverlapEvents(false);
	KeyLabel->SetHorizontalAlignment(EHTA_Center);
	KeyLabel->SetVerticalAlignment(EVRTA_TextCenter);
	KeyLabel->SetTextRenderColor(LabelColor);
	KeyLabel->SetWorldSize(LabelWorldSize);
	KeyLabel->SetRelativeLocation(LabelRelativeLocation);
	KeyLabel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	KeyLabelText = FText::FromString(TEXT("KEY"));
}

void ARunnerKeyPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (KeyMeshAsset)
	{
		KeyMesh->SetStaticMesh(KeyMeshAsset);
	}

	ApplyVisualSettings();
	ConfigureLabel();
	UpdateCollectTriggerFromVisualSize();
}

void ARunnerKeyPickup::BeginPlay()
{
	Super::BeginPlay();

	ApplyVisualSettings();
	ConfigureLabel();
	UpdateCollectTriggerFromVisualSize();
	UpdateLabelFacingPlayer();

	CollectTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &ARunnerKeyPickup::OnCollectTriggerOverlapBegin);
	CollectTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARunnerKeyPickup::OnCollectTriggerOverlapBegin);
}

void ARunnerKeyPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLabelFacingPlayer();
}

void ARunnerKeyPickup::RefreshKeyShape()
{
	SetActorScale3D(FVector::OneVector);
	ApplyVisualSettings();
	ConfigureLabel();
	UpdateCollectTriggerFromVisualSize();
}

void ARunnerKeyPickup::ApplyVisualSettings()
{
	if (KeyMesh == nullptr)
	{
		return;
	}

	bMakeKeyCubeLike = false;
	bShowLabel = false;

	SetActorScale3D(FVector::OneVector);
	KeyMesh->SetRelativeLocation(FVector::ZeroVector);
	KeyMesh->SetRelativeRotation(FRotator::ZeroRotator);
	UMaterialInterface* MaterialToKeep = KeyMaterial ? KeyMaterial.Get() : KeyMesh->GetMaterial(0);

	if (UStaticMesh* FishMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Models/Trout.Trout")))
	{
		KeyMeshAsset = FishMesh;
		KeyMesh->SetStaticMesh(KeyMeshAsset);
	}
	else if (KeyMeshAsset != nullptr)
	{
		KeyMesh->SetStaticMesh(KeyMeshAsset);
	}

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents(StaticMeshComponents);
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (!IsValid(StaticMeshComponent) || StaticMeshComponent == KeyMesh)
		{
			continue;
		}

		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetGenerateOverlapEvents(false);
		StaticMeshComponent->SetVisibility(false, true);
		StaticMeshComponent->SetHiddenInGame(true, true);
	}

	const UStaticMesh* StaticMesh = KeyMesh->GetStaticMesh();
	if (StaticMesh == nullptr)
	{
		return;
	}

	const FVector MeshSize = StaticMesh->GetBounds().BoxExtent * 2.0f;
	const float TargetSize = FMath::Max(FMath::Max3(KeyVisualSize.X, KeyVisualSize.Y, KeyVisualSize.Z), 1.0f);

	if (bMakeKeyCubeLike)
	{
		KeyVisualSize = FVector(170.0f, 170.0f, 8.0f);
		const FVector DesiredSize(KeyVisualSize);
		const FVector NewScale(
			MeshSize.X > UE_KINDA_SMALL_NUMBER ? DesiredSize.X / MeshSize.X : 1.0f,
			MeshSize.Y > UE_KINDA_SMALL_NUMBER ? DesiredSize.Y / MeshSize.Y : 1.0f,
			MeshSize.Z > UE_KINDA_SMALL_NUMBER ? DesiredSize.Z / MeshSize.Z : 1.0f);

		KeyMesh->SetRelativeScale3D(NewScale);
		ApplyKeyMaterial(MaterialToKeep);
		return;
	}

	const float MeshLargestSide = FMath::Max3(MeshSize.X, MeshSize.Y, MeshSize.Z);
	const float UniformScale = MeshLargestSide > UE_KINDA_SMALL_NUMBER ? TargetSize / MeshLargestSide : 1.0f;
	KeyMesh->SetRelativeScale3D(FVector(UniformScale));

	if (KeyMaterial)
	{
		ApplyKeyMaterial(KeyMaterial.Get());
	}
	else
	{
		KeyMesh->EmptyOverrideMaterials();
	}
}

void ARunnerKeyPickup::ApplyKeyMaterial(UMaterialInterface* PreferredMaterial)
{
	if (KeyMesh == nullptr)
	{
		return;
	}

	UMaterialInterface* MaterialToApply = KeyMaterial.Get();
	if (MaterialToApply == nullptr && PreferredMaterial != nullptr && !PreferredMaterial->GetPathName().Contains(TEXT("/Engine/BasicShapes/BasicShapeMaterial")))
	{
		MaterialToApply = PreferredMaterial;
	}

	if (MaterialToApply != nullptr)
	{
		KeyMesh->SetMaterial(0, MaterialToApply);
		return;
	}

	if (!bMakeKeyCubeLike)
	{
		return;
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* FallbackMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (FallbackMaterial == nullptr)
	{
		return;
	}

	const FLinearColor KeyPink(1.0f, 0.0f, 0.75f, 1.0f);
	FallbackMaterial->SetVectorParameterValue(TEXT("Color"), KeyPink);
	FallbackMaterial->SetVectorParameterValue(TEXT("BaseColor"), KeyPink);
	FallbackMaterial->SetVectorParameterValue(TEXT("Base Color"), KeyPink);
	KeyMesh->SetMaterial(0, FallbackMaterial);
}

void ARunnerKeyPickup::ConfigureLabel()
{
	if (KeyLabel == nullptr)
	{
		return;
	}

	bLabelFacesPlayer = false;
	bShowLabel = false;
	LabelRelativeLocation = FVector(0.0f, 0.0f, 7.0f);
	KeyLabel->SetText(KeyLabelText);
	KeyLabel->SetRelativeLocation(LabelRelativeLocation);
	KeyLabel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	KeyLabel->SetWorldSize(LabelWorldSize);
	KeyLabel->SetTextRenderColor(LabelColor);
	KeyLabel->SetHiddenInGame(!bShowLabel);
	KeyLabel->SetVisibility(bShowLabel, true);
}

void ARunnerKeyPickup::UpdateLabelFacingPlayer()
{
	if (!bShowLabel || !bLabelFacesPlayer || KeyLabel == nullptr)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn == nullptr)
	{
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - KeyLabel->GetComponentLocation();
	ToPlayer.Z = 0.0f;

	if (!ToPlayer.Normalize())
	{
		return;
	}

	KeyLabel->SetWorldRotation(FRotationMatrix::MakeFromX(ToPlayer).Rotator());
}

void ARunnerKeyPickup::UpdateCollectTriggerFromVisualSize()
{
	if (CollectTrigger == nullptr)
	{
		return;
	}

	if (bMakeKeyCubeLike || KeyMesh == nullptr || KeyMesh->GetStaticMesh() == nullptr)
	{
		const FVector HalfSize(
			FMath::Max((KeyVisualSize.X * 0.5f) + CollectTriggerPadding.X, 1.0f),
			FMath::Max((KeyVisualSize.Y * 0.5f) + CollectTriggerPadding.Y, 1.0f),
			FMath::Max((KeyVisualSize.Z * 0.5f) + CollectTriggerPadding.Z, 1.0f));

		CollectTrigger->SetRelativeLocation(FVector::ZeroVector);
		CollectTrigger->SetBoxExtent(HalfSize, true);
		return;
	}

	const FBoxSphereBounds MeshBounds = KeyMesh->GetStaticMesh()->GetBounds();
	const FVector MeshScale = KeyMesh->GetRelativeScale3D().GetAbs();
	const FVector MeshCenter = KeyMesh->GetRelativeTransform().TransformPosition(MeshBounds.Origin);
	const FVector ScaledMeshExtent = MeshBounds.BoxExtent * MeshScale;
	const FVector HalfSize(
		FMath::Max(ScaledMeshExtent.X + CollectTriggerPadding.X, 1.0f),
		FMath::Max(ScaledMeshExtent.Y + CollectTriggerPadding.Y, 1.0f),
		FMath::Max(ScaledMeshExtent.Z + CollectTriggerPadding.Z, 1.0f));

	CollectTrigger->SetRelativeLocation(MeshCenter);
	CollectTrigger->SetBoxExtent(HalfSize, true);
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

	KeyValue = 2;
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

	OnKeyCollected.Broadcast(Runner, KeyValue, this);
	BP_OnKeyCollected(Runner, KeyValue, this);

	if (Runner->AddScore(KeyValue))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Runner score updated to %d."), Runner->GetScore());
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

