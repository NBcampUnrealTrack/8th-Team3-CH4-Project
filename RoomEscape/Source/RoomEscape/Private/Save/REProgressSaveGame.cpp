#include "Save/REProgressSaveGame.h"

UREProgressSaveGame::UREProgressSaveGame()
{
	CreatedAtUtc = FDateTime::UtcNow().ToIso8601();
	LastSavedAtUtc = CreatedAtUtc;
}

bool UREProgressSaveGame::HasSolvedPuzzle(FName PuzzleId) const
{
	return PuzzleId.IsNone() == false && SolvedPuzzleIds.Contains(PuzzleId) == true;
}

bool UREProgressSaveGame::AddSolvedPuzzle(FName PuzzleId)
{
	if (PuzzleId.IsNone() == true || SolvedPuzzleIds.Contains(PuzzleId) == true)
	{
		return false;
	}

	SolvedPuzzleIds.Add(PuzzleId);
	return true;
}

bool UREProgressSaveGame::RemoveSolvedPuzzle(FName PuzzleId)
{
	return PuzzleId.IsNone() == false && SolvedPuzzleIds.Remove(PuzzleId) > 0;
}

void UREProgressSaveGame::ClearProgress()
{
	SolvedPuzzleIds.Reset();
	ClearCheckpoint();
}

void UREProgressSaveGame::SetCheckpoint(FName CheckpointId, const FTransform& CheckpointTransform)
{
	LastCheckpoint.CheckpointId = CheckpointId;
	LastCheckpoint.Transform = CheckpointTransform;
	LastCheckpoint.bHasValidTransform = true;
}

void UREProgressSaveGame::ClearCheckpoint()
{
	LastCheckpoint.CheckpointId = NAME_None;
	LastCheckpoint.Transform = FTransform::Identity;
	LastCheckpoint.bHasValidTransform = false;
}

void UREProgressSaveGame::TouchLastSavedAt()
{
	LastSavedAtUtc = FDateTime::UtcNow().ToIso8601();
}
