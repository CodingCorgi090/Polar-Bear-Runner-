#pragma once

#include "FileHandlers.generated.h"

/**
 * Lightweight C++ base widget for runner health/game-over UI.
 * Implement the BP events in a Widget Blueprint for visuals.
 */
UCLASS(Blueprintable, BlueprintType)
class POLAR_BEAR_RUNNER_API UFile_Handler: public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FileHandler")
	UFile_Handler* FileHandler = nullptr;
	
	UFUNCTION(BlueprintCallable, Category="FileHandler")
	UFile_Handler* GetFileHandler() const { return FileHandler; }
	
	UFUNCTION(BlueprintCallable, Category="FileHandler")
	TArray<FString> GetScores();
	
	UFUNCTION(BlueprintCallable, Category="FileHandler")
	bool SaveScores(FString User, int32 const Score);

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FileHandler")
	FString ScoreDirectory = "";
	
	
};
