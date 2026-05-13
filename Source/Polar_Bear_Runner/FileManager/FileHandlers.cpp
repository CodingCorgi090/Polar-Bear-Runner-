

#include "FileHandlers.h"
#include "Polar_Bear_Runner.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

/** File handler managers reading and writing to external 
 text files. */

// Saves a score to an external text file
bool UFile_Handler::SaveScores(int32 const Score, FString const User)
{
	// Define the path where the score is to be saved
	FString const HighScorePath = FPaths::ProjectDir() + ScoreDirectory;
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *HighScorePath);
	
	// Converts the score argument into a string and defines a local
	// variable for the user
	// Define other needed variables
	FString const UserScore = FString::FromInt(Score);
	FString const UserName = User;
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	FString SavedScores;
	
	// Compile the User and score into one string to write to the file
	FString HighScoreEntry = FString::Printf(TEXT("%s;%s"), *UserName, *UserScore);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *HighScoreEntry);
	
	// Check if the file exists yet
	// If the file doesn't exist, just write the current score to the file
	// this will create the file and add the first score
	if (!File.FileExists(*HighScorePath))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("No Score Directory. Creating one..."));
		return FFileHelper::SaveStringToFile(*HighScoreEntry, *HighScorePath);
	}
	
	// If the file does exist, read the file and store the returned string to a variable
	bool bScoresReceived = FFileHelper::LoadFileToString(SavedScores, *HighScorePath);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *SavedScores);
	
	// If the file fails to be read log a message and return
	if (!bScoresReceived)
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("The score file should exist but there is an error reading it..."));
		return false;
	}
	
	// Add the new score entry to the bottom of the score list
	// on a newline
	FString NewSavedScores = FString::Printf(TEXT("%s\n%s"), *SavedScores, *HighScoreEntry);
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("%s"), *NewSavedScores);
		
	// Write the new score list to the file
	bool bScoresSaved = FFileHelper::SaveStringToFile(*NewSavedScores, *HighScorePath);
		
	// Log a message if the write failed
	if (!bScoresSaved)
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Write failed."));
		return false;
	}
		
	// Log a message indicating that the write was successful
	UE_LOG(LogPolar_Bear_Runner, Log, TEXT("Write complete."));
	return true;
}

// Reads the score list from an external file, parses the list into an array
// using a newline as the delimiter, and then returns the array of strings.
TArray<FString> UFile_Handler::GetScores()
{
	// Defines the path from where the score is to be retrieved
	FString const HighScorePath = FPaths::ProjectDir() + ScoreDirectory;
	
	// Initialize variables
	FString SavedScores;
	TArray<FString> HighScoreData;
	IPlatformFile& ReadFile = FPlatformFileManager::Get().GetPlatformFile();
	
	// Check if the file exists yet
	// If the file doesn't exist, return an empty array
	if (!ReadFile.FileExists(*HighScorePath))
	{
		UE_LOG(LogPolar_Bear_Runner, Log, TEXT("A score directory doesn't exist."));
		return TArray<FString>();
	}
	
	// Read the scores from the file
	bool bScoresReceived = FFileHelper::LoadFileToString(SavedScores, *HighScorePath);
	
	// If no string is received, return an array containing one 
	// string that informs that there are no scores
	if (!bScoresReceived)
	{	
		TArray<FString> NoScores;
		NoScores.Emplace(TEXT("No Saved Scores!"));
		return NoScores;
	}
	
	// Parse the score string into an array using a newline as the delimiter
	SavedScores.ParseIntoArray(HighScoreData, TEXT("\n"), true);
	
	// Return the array of scores
	return HighScoreData;
}

