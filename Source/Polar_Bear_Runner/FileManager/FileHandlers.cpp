#include "FileHandlers.h"

#include "Polar_Bear_Runner.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"


bool UFile_Handler::SaveScores(FString const User, int32 const Score)
{
	
	//FString const HighScorePath = FPaths::ProjectDir() + TEXT("Intermediate/ProjectFiles/ScoreResults/HighScores.txt");
	FString const HighScorePath = FPaths::ProjectDir() + TEXT("HighScores.txt");
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *HighScorePath);
	FString const UserScore = FString::FromInt(Score);
	FString const UserName = User;
	FString SavedScores;
	
	FString HighScoreEntry = FString::Printf(TEXT("%s: %s"), *UserName, *UserScore);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *HighScoreEntry);
	
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	
	if (!File.FileExists(*HighScorePath))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("No Score Directory. Creating one..."));
		return FFileHelper::SaveStringToFile(*HighScoreEntry, *HighScorePath);
	}
	
	bool bScoresReceived = FFileHelper::LoadFileToString(SavedScores, *HighScorePath);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *SavedScores);
	if (bScoresReceived)
	{
		FString NewSavedScores = FString::Printf(TEXT("%s\n%s"), *SavedScores, *HighScoreEntry);
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *NewSavedScores);
		bool bScoresSaved = FFileHelper::SaveStringToFile(*NewSavedScores, *HighScorePath);
		
		if (!bScoresSaved)
		{
			UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Write complete"));
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

TArray<FString> UFile_Handler::GetScores()
{
	FString const HighScorePath = FPaths::ProjectDir() + TEXT("HighScores.txt");
	FString SavedScores;
	TArray<FString> HighScoreData;
	
	bool bScoresReceived = FFileHelper::LoadFileToString(SavedScores, *HighScorePath);
	
	if (!bScoresReceived)
	{	
		TArray<FString> NoScores;
		NoScores.Emplace(TEXT("No Saved Scores!"));
		return NoScores;
	}
	
	SavedScores.ParseIntoArray(HighScoreData, TEXT(","), true);
	return HighScoreData;
}

