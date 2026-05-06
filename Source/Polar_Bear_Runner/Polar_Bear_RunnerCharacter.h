// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Polar_Bear_RunnerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

// Forward declare the spawn point actor so designers can assign it in Blueprints
class ARunnerSpawnPoint;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class ERunnerDamageType : uint8
{
	MissedKey UMETA(DisplayName = "Missed Key"),
	ObstacleHit UMETA(DisplayName = "Obstacle Hit"),
	Custom UMETA(DisplayName = "Custom")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FRunnerDamageTakenSignature, float, DamageAmount, float, NewHealth, float, MaxHealth, ERunnerDamageType, DamageType, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRunnerHealthChangedSignature, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRunnerDiedSignature, ERunnerDamageType, DamageType, AActor*, DamageCauser);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class APolar_Bear_RunnerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	APolar_Bear_RunnerCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Applies damage with shared guardrails (cooldown, dead-state checks, clamping). */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual bool ApplyRunnerDamage(float DamageAmount, ERunnerDamageType DamageType, AActor* DamageCauser = nullptr);

	/** Applies default (or overridden) damage for a missed musical key. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual bool RequestDamageFromMissedKey(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Kills the runner for obstacle hits. DamageOverride is ignored so obstacle contact is always lethal. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual bool RequestDamageFromObstacle(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Kills the runner immediately, bypassing damage cooldown. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual bool KillRunner(ERunnerDamageType DamageType, AActor* DamageCauser = nullptr);

	/** Resets health (and optionally revive state) between runs/checkpoints. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual void ResetRunnerHealth(bool bRevive = true);

	/** Respawns the player at the initial location, resetting health and state. */
	UFUNCTION(BlueprintCallable, Category="Runner|Respawn")
	virtual void RespawnPlayer();

	/** Sets the actor used as this runner's respawn location. */
	UFUNCTION(BlueprintCallable, Category="Runner|Respawn")
	virtual void SetRespawnPoint(ARunnerSpawnPoint* NewSpawnPoint);

	/** Returns the currently assigned respawn point, if any. */
	UFUNCTION(BlueprintPure, Category="Runner|Respawn")
	ARunnerSpawnPoint* GetRespawnPoint() const { return AssignedSpawnPoint; }

	/** Returns current normalized health from 0.0 to 1.0. */
	UFUNCTION(BlueprintPure, Category="Runner|Damage")
	virtual float GetHealthPercent() const;

	/** Returns current raw health value. */
	UFUNCTION(BlueprintPure, Category="Runner|Damage")
	virtual float GetCurrentHealth() const { return CurrentHealth; }

	/** Returns max health value. */
	UFUNCTION(BlueprintPure, Category="Runner|Damage")
	virtual float GetMaxHealthValue() const { return MaxHealth; }

	/** Returns true once the runner has reached zero health. */
	UFUNCTION(BlueprintPure, Category="Runner|Damage")
	virtual bool IsRunnerDead() const { return bIsDead; }

	/** Fired after damage is successfully applied. */
	UPROPERTY(BlueprintAssignable, Category="Runner|Events")
	FRunnerDamageTakenSignature OnRunnerDamageTaken;

	/** Fired whenever health changes. */
	UPROPERTY(BlueprintAssignable, Category="Runner|Events")
	FRunnerHealthChangedSignature OnRunnerHealthChanged;

	/** Fired once when health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category="Runner|Events")
	FRunnerDiedSignature OnRunnerDied;

	/** BP hook for VFX/SFX/UI when damage is applied. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Events")
	void BP_OnRunnerDamageTaken(float DamageAmount, float NewHealth, float MaxHealthValue, ERunnerDamageType DamageType, AActor* DamageCauser);

	/** BP hook for UI health bars and widgets. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Events")
	void BP_OnRunnerHealthChanged(float NewHealth, float MaxHealthValue);

	/** BP hook for game over flow. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Events")
	void BP_OnRunnerDied(ERunnerDamageType DamageType, AActor* DamageCauser);
	
	//Adds the increment amount to the current score
	UFUNCTION(BlueprintCallable, Category = "Runner|Score")
	bool AddScore(int32 const Amount);
	
	/** BP hook for game over flow. */
	UFUNCTION(BlueprintImplementableEvent, Category="Runner|Events")
	void BP_OnScoreChanged(int NewScore);
	
	//Returns the current score
	UFUNCTION(BlueprintCallable, Category = "Runner|Score")
	int GetScore() const;

	//Resets the score... may be unnecessary at this stage
	UFUNCTION(BlueprintCallable, Category = "Runner|Score")
	void ResetScore();

protected:
	/** Max health for each run. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage", meta=(ClampMin="1.0", UIMin="1.0"))
	float MaxHealth = 100.0f;

	/** Runtime health value used by UI/game over checks. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Damage", meta=(ClampMin="0.0", UIMin="0.0"))
	float CurrentHealth = 100.0f;

	/** Damage applied when a key note is missed and no override is provided. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage", meta=(ClampMin="0.0", UIMin="0.0"))
	float MissedKeyDamage = 10.0f;

	/** If true, repeated damage can be throttled by DamageCooldownSeconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage")
	bool bUseDamageCooldown = false;

	/** Optional grace period to avoid duplicate hit events in the same instant. Ignored unless Use Damage Cooldown is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage",
	          meta=(EditCondition="bUseDamageCooldown", ClampMin="0.0", UIMin="0.0"))
	float DamageCooldownSeconds = 0.0f;

	/** True once runner has no health left. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Damage")
	bool bIsDead = false;

	//Player score
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runner|Score")
	int Score = 0;
	/** Optional spawn point reference. Assign this on the Character Blueprint or placed instance to control respawn.
	 *  If left unset, the runner respawns at its starting transform.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Runner|Respawn", meta=(AllowPrivateAccess="true"))
	TObjectPtr<ARunnerSpawnPoint> AssignedSpawnPoint = nullptr;

	/** Initial transform for respawning (cached). */
	FTransform InitialTransform;

private:
	float LastDamageTimeSeconds = -1.0f;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

