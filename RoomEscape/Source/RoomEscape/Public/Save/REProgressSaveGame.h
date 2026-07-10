#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "REProgressSaveGame.generated.h"

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FRECheckpointSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save|Checkpoint")
	FName CheckpointId = NAME_None;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save|Checkpoint")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save|Checkpoint")
	bool bHasValidTransform = false;
};

UCLASS(BlueprintType)
class ROOMESCAPE_API UREProgressSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UREProgressSaveGame();

public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	int32 SlotIndex = 0;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	FString SlotName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	FString CreatedAtUtc;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
	FString LastSavedAtUtc;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save|Progress")
	TArray<FName> SolvedPuzzleIds;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save|Progress")
	FRECheckpointSaveData LastCheckpoint;

public:
	UFUNCTION(BlueprintPure, Category = "Save|Progress")
	bool HasSolvedPuzzle(FName PuzzleId) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool AddSolvedPuzzle(FName PuzzleId);

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool RemoveSolvedPuzzle(FName PuzzleId);

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	void ClearProgress();

	UFUNCTION(BlueprintCallable, Category = "Save|Checkpoint")
	void SetCheckpoint(FName CheckpointId, const FTransform& CheckpointTransform);

	UFUNCTION(BlueprintCallable, Category = "Save|Checkpoint")
	void ClearCheckpoint();

	void TouchLastSavedAt();
};
