// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Polar_Bear_RunnerPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class AActor;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class APolar_Bear_RunnerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:
	/** Routes missed-key damage to the currently possessed runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportMissedKeyDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

	/** Routes obstacle-hit damage to the currently possessed runner character. */
	UFUNCTION(BlueprintCallable, Category="Runner|Damage")
	bool ReportObstacleDamage(float DamageOverride = -1.0f, AActor* DamageCauser = nullptr);

};
