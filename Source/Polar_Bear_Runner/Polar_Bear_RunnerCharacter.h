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

	/** Applies default (or overridden) damage for hitting an obstacle. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual bool RequestDamageFromObstacle(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Resets health (and optionally revive state) between runs/checkpoints. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	virtual void ResetRunnerHealth(bool bRevive = true);

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

	/** Damage applied when colliding with obstacles and no override is provided. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage", meta=(ClampMin="0.0", UIMin="0.0"))
	float ObstacleHitDamage = 20.0f;

	/** Small grace period to avoid duplicate hit events in the same instant. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Runner|Damage", meta=(ClampMin="0.0", UIMin="0.0"))
	float DamageCooldownSeconds = 0.1f;

	/** True once runner has no health left. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Runner|Damage")
	bool bIsDead = false;

private:
	float LastDamageTimeSeconds = -1.0f;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

