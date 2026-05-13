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
#include "Polar_Bear_RunnerPlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "RunnerEndlessCourse.h"
#include "RunnerSpawnPoint.h"

APolar_Bear_RunnerCharacter::APolar_Bear_RunnerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
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

APolar_Bear_RunnerPlayerController* APolar_Bear_RunnerCharacter::GetPolarBearController()
{
	APolar_Bear_RunnerPlayerController* PolarBearController = Cast<APolar_Bear_RunnerPlayerController>(this->GetController());
	return PolarBearController;
}
void APolar_Bear_RunnerCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = AssignedSpawnPoint ? AssignedSpawnPoint->GetActorTransform() : GetActorTransform();
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

	CacheAutoRunForwardDirection(InitialTransform.Rotator());
	ResetRunnerAccel();
	GetHighScore();
	
	
}

void APolar_Bear_RunnerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAutoRunForward || bIsDead || GetCharacterMovement() == nullptr || GetCharacterMovement()->MovementMode == MOVE_None)
	{
		return;
	}

	AutoRunElapsedSeconds += FMath::Max(DeltaTime, 0.0f);

	const float TimeTargetSpeed = FMath::Max(AutoRunStartSpeed, 400.0f) + (AutoRunElapsedSeconds * FMath::Max(AutoRunAccelerationPerSecond, 10.0f));
	if (TimeTargetSpeed > NewWalkSpeed)
	{
		NewWalkSpeed = FMath::Min(TimeTargetSpeed, MaxAutoRunSpeed);
		ApplyCurrentRunSpeed();
	}

	AddMovementInput(AutoRunForwardDirection, 1.0f);
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
		// Logs the new score value
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Score changed. New score: %d"), Score);
		
		// Determines when the player levels up
		if (Score % 10 == 0)
		{
			AddPlayerLevel();
		}
		return true;
	}
	else {
		return false;
	}
/**
	const int32 PreviousScore = Score;
	Score += Amount;

	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Score changed. New score: %d"), Score);
	OnRunnerScoreChanged.Broadcast(Score, Amount);
	BP_OnScoreChanged(Score);

	const int32 ClampedPointsPerSpeedIncrease = FMath::Max(PointsPerSpeedIncrease, 1);
	const int32 PreviousSpeedTier = PreviousScore / ClampedPointsPerSpeedIncrease;
	const int32 NewSpeedTier = Score / ClampedPointsPerSpeedIncrease;

	for (int32 TierIndex = PreviousSpeedTier; TierIndex < NewSpeedTier; ++TierIndex)
	{
		AddPlayerLevel();
	}

	return true;
	*/
}

// Sets the score to 0 
void APolar_Bear_RunnerCharacter::ResetScore()
{
	const int32 PreviousScore = Score;
	Score = 0;

	if (PreviousScore != 0)
	{
		OnRunnerScoreChanged.Broadcast(Score, -PreviousScore);
		BP_OnScoreChanged(Score);
	}
}

void APolar_Bear_RunnerCharacter::RespawnPlayer()
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("RespawnPlayer called. Initial transform location: %s"), *InitialTransform.GetLocation().ToString());

	// Reset velocity
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->StopMovementImmediately();

	// Use the cached spawn transform so recycled/moved spawn actors cannot pull respawn away from the run start.
	FTransform SpawnTransform = InitialTransform;
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Respawning to cached spawn transform at %s"), *SpawnTransform.GetLocation().ToString());

	const FVector DesiredSpawnLocation = SpawnTransform.GetLocation();
	FRotator SpawnRotation = SpawnTransform.Rotator();

	SetActorEnableCollision(false);
	SetActorHiddenInGame(false);

	SetActorLocationAndRotation(DesiredSpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
	RebuildEndlessCoursesForRespawn();

	FVector SpawnLocation = GetGroundedRespawnLocation(DesiredSpawnLocation);
	if (SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Teleport successful to %s"), *GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogPolar_Bear_Runner, Warning, TEXT("Teleport failed! Setting transform directly."));
		SpawnTransform.SetLocation(SpawnLocation);
		SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	SetActorEnableCollision(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->StopMovementImmediately();

	// Reset health
	ResetRunnerHealth(true);
	ResetScore();
	ResetPlayerLevel();
	CacheAutoRunForwardDirection(SpawnRotation);
	ResetRunnerAccel();
	GetHighScore();

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

// Adds to the players level	
bool APolar_Bear_RunnerCharacter::AddPlayerLevel()
{
	PlayerLevel ++;
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Level changed. New level: %d"), PlayerLevel);
	
	if (GetPolarBearController())
	{
		GetPolarBearController()->ReportLevelUpdate(PlayerLevel);
	}
	
	ApplyRunnerAccel();
	return true;
	
}

// Returns the player level
int32 APolar_Bear_RunnerCharacter::GetPlayerLevel() const
{
	return PlayerLevel;
}

// Sets the level to 0 
void APolar_Bear_RunnerCharacter::ResetPlayerLevel()
{
	PlayerLevel = 0;
	
	if (GetPolarBearController())
	{
		GetPolarBearController()->ReportLevelUpdate(PlayerLevel);
	}
}


// used to apply runner accel on level up
bool APolar_Bear_RunnerCharacter::ApplyRunnerAccel()
{
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Current walk speed: %f"), NewWalkSpeed);
	if (NewWalkSpeed < MaxAutoRunSpeed)
	{
		NewWalkSpeed = FMath::Min(NewWalkSpeed + FMath::Max(ScoreSpeedIncrease, 60.0f), MaxAutoRunSpeed);
		//UE_LOG(LogPolar_Bear_Runner, Log, TEXT("New walk speed: %f"), NewWalkSpeed);

		ApplyCurrentRunSpeed();
		return true;
	}
	else
	{
		return false;
	}
}

// Sets the level to 0 
void APolar_Bear_RunnerCharacter::ResetRunnerAccel()
{
	AutoRunElapsedSeconds = 0.0f;
	NewWalkSpeed = FMath::Max(AutoRunStartSpeed, 400.0f);
	ApplyCurrentRunSpeed();
}

void APolar_Bear_RunnerCharacter::CacheAutoRunForwardDirection(const FRotator& ForwardRotation)
{
	AutoRunForwardDirection = ForwardRotation.Vector();
	AutoRunForwardDirection.Z = 0.0f;

	if (!AutoRunForwardDirection.Normalize())
	{
		AutoRunForwardDirection = FVector::ForwardVector;
	}
}

void APolar_Bear_RunnerCharacter::ApplyCurrentRunSpeed()
{
	NewWalkSpeed = FMath::Clamp(NewWalkSpeed, 1.0f, MaxAutoRunSpeed);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%f applied"), NewWalkSpeed);
	}
}

FVector APolar_Bear_RunnerCharacter::GetGroundedRespawnLocation(const FVector& DesiredLocation) const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return DesiredLocation;
	}

	const float CapsuleHalfHeight = GetCapsuleComponent()
		? GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		: 96.0f;

	const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight + 1000.0f);
	const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, 5000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RunnerRespawnGroundTrace), false, this);
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		return FVector(DesiredLocation.X, DesiredLocation.Y, HitResult.Location.Z + CapsuleHalfHeight + 2.0f);
	}

	return DesiredLocation;
}

void APolar_Bear_RunnerCharacter::RebuildEndlessCoursesForRespawn() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<ARunnerEndlessCourse> CourseIt(World); CourseIt; ++CourseIt)
	{
		if (ARunnerEndlessCourse* Course = *CourseIt)
		{
			Course->RebuildCourse();
		}
	}
}

// Reads the scores from the text file, parses them and sets the high score on the HUD
void APolar_Bear_RunnerCharacter::GetHighScore()
{
	// Set up variables
	UFile_Handler* FileHandler = NewObject<UFile_Handler>(this);
	TArray<FString> MyScores = FileHandler->GetScores();
	int32 HighScore = 0;
	
	// Verify that the returned value is an array
	if (MyScores.Num() >= 1)
	{
		// Loop through the array values
		for (int32 index = 0; index < MyScores.Num(); ++index)
		{
			// Parse the user/score value on the delimiter
			TArray<FString> ScoreDataArray;
			MyScores[index].ParseIntoArray(ScoreDataArray, TEXT(";"), true);
			
			// Verify that the new array contains a score value
			if (ScoreDataArray.Num() >= 1 )
			{
				// Convert the string score value into an integer
				int32 ScoreNum = FCString::Atoi(*ScoreDataArray[1]);
				UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%d"), ScoreNum);
				
				// Compare the value to the current high score value and update if the new score is greater
				if (ScoreNum > HighScore)
				{
					HighScore = ScoreNum;
					if (GetPolarBearController())
					{
						GetPolarBearController()->ReportHighScoreUpdate(HighScore);
					}
				}
			}
		}
	}
	else
	{
		GetPolarBearController()->ReportHighScoreUpdate(0);
	}
}