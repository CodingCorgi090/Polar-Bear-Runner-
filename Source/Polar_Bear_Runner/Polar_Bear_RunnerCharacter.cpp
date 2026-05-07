// Copyright Epic Games, Inc. All Rights Reserved.

#include "Polar_Bear_RunnerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Polar_Bear_Runner.h"
#include "Engine/World.h"
#include "RunnerSpawnPoint.h"

APolar_Bear_RunnerCharacter::APolar_Bear_RunnerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void APolar_Bear_RunnerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ResetRunnerHealth(true);

	InitialTransform = GetActorTransform();

	if (AssignedSpawnPoint != nullptr)
	{
		InitialTransform = AssignedSpawnPoint->GetActorTransform();
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Using AssignedSpawnPoint '%s' at %s"), *GetNameSafe(AssignedSpawnPoint), *InitialTransform.GetLocation().ToString());
		SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("No AssignedSpawnPoint set. Using character start transform at %s"), *InitialTransform.GetLocation().ToString());
	}

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Character initial transform set to %s"), *InitialTransform.GetLocation().ToString());
}

void APolar_Bear_RunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APolar_Bear_RunnerCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APolar_Bear_RunnerCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APolar_Bear_RunnerCharacter::Look);
	}
	else
	{
		UE_LOG(LogPolar_Bear_Runner, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APolar_Bear_RunnerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void APolar_Bear_RunnerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APolar_Bear_RunnerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void APolar_Bear_RunnerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APolar_Bear_RunnerCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void APolar_Bear_RunnerCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

bool APolar_Bear_RunnerCharacter::ApplyRunnerDamage(float DamageAmount, ERunnerDamageType DamageType, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || bIsDead)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.0f;

	if (bUseDamageCooldown && DamageCooldownSeconds > 0.0f && LastDamageTimeSeconds >= 0.0f)
	{
		if (TimeSeconds - LastDamageTimeSeconds < DamageCooldownSeconds)
		{
			return false;
		}
	}

	LastDamageTimeSeconds = TimeSeconds;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	OnRunnerHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	BP_OnRunnerHealthChanged(CurrentHealth, MaxHealth);

	OnRunnerDamageTaken.Broadcast(DamageAmount, CurrentHealth, MaxHealth, DamageType, DamageCauser);
	BP_OnRunnerDamageTaken(DamageAmount, CurrentHealth, MaxHealth, DamageType, DamageCauser);

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnRunnerDied.Broadcast(DamageType, DamageCauser);
		BP_OnRunnerDied(DamageType, DamageCauser);
	}

	return true;
}

bool APolar_Bear_RunnerCharacter::RequestDamageFromMissedKey(float DamageOverride, AActor* DamageCauser)
{
	const float DamageToApply = DamageOverride > 0.0f ? DamageOverride : MissedKeyDamage;
	return ApplyRunnerDamage(DamageToApply, ERunnerDamageType::MissedKey, DamageCauser);
}

bool APolar_Bear_RunnerCharacter::RequestDamageFromObstacle(float DamageOverride, AActor* DamageCauser)
{
	(void)DamageOverride;
	return KillRunner(ERunnerDamageType::ObstacleHit, DamageCauser);
}

bool APolar_Bear_RunnerCharacter::KillRunner(ERunnerDamageType DamageType, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return false;
	}

	bIsDead = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	const UWorld* World = GetWorld();
	LastDamageTimeSeconds = World ? World->GetTimeSeconds() : 0.0f;

	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	const float DamageAmount = FMath::Max(CurrentHealth, MaxHealth);
	CurrentHealth = 0.0f;

	OnRunnerHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	BP_OnRunnerHealthChanged(CurrentHealth, MaxHealth);

	OnRunnerDamageTaken.Broadcast(DamageAmount, CurrentHealth, MaxHealth, DamageType, DamageCauser);
	BP_OnRunnerDamageTaken(DamageAmount, CurrentHealth, MaxHealth, DamageType, DamageCauser);

	OnRunnerDied.Broadcast(DamageType, DamageCauser);
	BP_OnRunnerDied(DamageType, DamageCauser);

	return true;
}

void APolar_Bear_RunnerCharacter::ResetRunnerHealth(bool bRevive)
{
	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	CurrentHealth = MaxHealth;
	LastDamageTimeSeconds = -1.0f;

	if (bRevive)
	{
		bIsDead = false;
	}

	OnRunnerHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	BP_OnRunnerHealthChanged(CurrentHealth, MaxHealth);
}

float APolar_Bear_RunnerCharacter::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

// Return the player score
int APolar_Bear_RunnerCharacter::GetScore() const
{
	return Score;
}

// Adds the increment to the current score and saves the result
// as the new score
bool APolar_Bear_RunnerCharacter::AddScore(int32 const Amount)
{
	// Verify that there is a positive increment, then calculate
	if (Amount >= 1) {
		Score += Amount;
		return true;
	}
	else {
		return false;
	}
}

// Sets the score to 0 
void APolar_Bear_RunnerCharacter::ResetScore()
{
	Score = 0;
}

void APolar_Bear_RunnerCharacter::RespawnPlayer()
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("RespawnPlayer called. Initial transform location: %s"), *InitialTransform.GetLocation().ToString());

	// Ensure the character is enabled
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Reset velocity
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->StopMovementImmediately();

	// Use TeleportTo for proper character movement component handling. If an AssignedSpawnPoint exists use its current transform
	FTransform SpawnTransform = InitialTransform;
	if (AssignedSpawnPoint != nullptr)
	{
		SpawnTransform = AssignedSpawnPoint->GetActorTransform();
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Respawning to AssignedSpawnPoint '%s' at %s"), *GetNameSafe(AssignedSpawnPoint), *SpawnTransform.GetLocation().ToString());
	}

	FVector SpawnLocation = SpawnTransform.GetLocation();
	FRotator SpawnRotation = SpawnTransform.Rotator();

	if (TeleportTo(SpawnLocation, SpawnRotation))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Teleport successful to %s"), *GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("Teleport failed! Setting transform directly."));
		SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// Reset health
	ResetRunnerHealth(true);

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("RespawnPlayer complete. Current location: %s, Health: %f"), *GetActorLocation().ToString(), CurrentHealth);
}

void APolar_Bear_RunnerCharacter::SetRespawnPoint(ARunnerSpawnPoint* NewSpawnPoint)
{
	AssignedSpawnPoint = NewSpawnPoint;

	if (AssignedSpawnPoint != nullptr)
	{
		InitialTransform = AssignedSpawnPoint->GetActorTransform();
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("AssignedSpawnPoint changed to '%s' at %s"),
		       *GetNameSafe(AssignedSpawnPoint), *InitialTransform.GetLocation().ToString());
		return;
	}

	InitialTransform = GetActorTransform();
}

