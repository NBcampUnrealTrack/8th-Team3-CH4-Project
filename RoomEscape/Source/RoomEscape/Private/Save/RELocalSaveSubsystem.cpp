#include "Save/RELocalSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Save/REProgressSaveGame.h"

void URELocalSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSlot(ActiveSlotIndex);
}

void URELocalSaveSubsystem::Deinitialize()
{
	ActiveSaveGame = nullptr;
	Super::Deinitialize();
}

bool URELocalSaveSubsystem::SetActiveSlotIndex(int32 NewSlotIndex, bool bLoadImmediately)
{
	const int32 SanitizedSlotIndex = FMath::Max(0, NewSlotIndex);
	if (ActiveSlotIndex == SanitizedSlotIndex && IsValid(ActiveSaveGame) == true)
	{
		return true;
	}

	ActiveSlotIndex = SanitizedSlotIndex;
	OnActiveSlotChanged.Broadcast(ActiveSlotIndex);

	if (bLoadImmediately == true)
	{
		return LoadSlot(ActiveSlotIndex);
	}

	ActiveSaveGame = nullptr;
	return true;
}

int32 URELocalSaveSubsystem::GetActiveSlotIndex() const
{
	return ActiveSlotIndex;
}

FString URELocalSaveSubsystem::GetActiveSlotName() const
{
	return BuildSlotName(ActiveSlotIndex);
}

FString URELocalSaveSubsystem::BuildSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("%s_%02d"), *SlotNamePrefix, FMath::Max(0, SlotIndex));
}

bool URELocalSaveSubsystem::DoesSlotExist(int32 SlotIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(BuildSlotName(SlotIndex), UserIndex);
}

void URELocalSaveSubsystem::GetExistingSlotIndices(int32 MaxSlotCount, TArray<int32>& OutSlotIndices) const
{
	OutSlotIndices.Reset();
	const int32 SanitizedMaxSlotCount = FMath::Max(0, MaxSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < SanitizedMaxSlotCount; ++SlotIndex)
	{
		if (DoesSlotExist(SlotIndex) == true)
		{
			OutSlotIndices.Add(SlotIndex);
		}
	}
}

bool URELocalSaveSubsystem::LoadSlot(int32 SlotIndex)
{
	ActiveSlotIndex = FMath::Max(0, SlotIndex);
	const FString SlotName = BuildSlotName(ActiveSlotIndex);

	UREProgressSaveGame* LoadedSaveGame = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) == true)
	{
		LoadedSaveGame = Cast<UREProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	}

	if (IsValid(LoadedSaveGame) == false)
	{
		LoadedSaveGame = CreateEmptySaveGame(ActiveSlotIndex);
	}

	ActiveSaveGame = LoadedSaveGame;
	OnSaveLoaded.Broadcast();
	return IsValid(ActiveSaveGame) == true;
}

bool URELocalSaveSubsystem::SaveActiveSlot()
{
	return PersistActiveSaveGame();
}

bool URELocalSaveSubsystem::DeleteSlot(int32 SlotIndex)
{
	const int32 SanitizedSlotIndex = FMath::Max(0, SlotIndex);
	const FString SlotName = BuildSlotName(SanitizedSlotIndex);
	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex) == false)
	{
		return true;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	if (bDeleted == true && SanitizedSlotIndex == ActiveSlotIndex)
	{
		LoadSlot(ActiveSlotIndex);
	}

	return bDeleted;
}

bool URELocalSaveSubsystem::MarkPuzzleSolved(FName PuzzleId, bool bSaveImmediately)
{
	UREProgressSaveGame* SaveGame = EnsureActiveSaveGame();
	if (IsValid(SaveGame) == false || PuzzleId.IsNone() == true)
	{
		return false;
	}

	const bool bChanged = SaveGame->AddSolvedPuzzle(PuzzleId);
	if (bChanged == true)
	{
		OnSaveProgressChanged.Broadcast();
		if (bSaveImmediately == true)
		{
			return PersistActiveSaveGame();
		}
	}

	return true;
}

bool URELocalSaveSubsystem::UnmarkPuzzleSolved(FName PuzzleId, bool bSaveImmediately)
{
	UREProgressSaveGame* SaveGame = EnsureActiveSaveGame();
	if (IsValid(SaveGame) == false || PuzzleId.IsNone() == true)
	{
		return false;
	}

	const bool bChanged = SaveGame->RemoveSolvedPuzzle(PuzzleId);
	if (bChanged == true)
	{
		OnSaveProgressChanged.Broadcast();
		if (bSaveImmediately == true)
		{
			return PersistActiveSaveGame();
		}
	}

	return true;
}

bool URELocalSaveSubsystem::IsPuzzleSolved(FName PuzzleId) const
{
	return IsValid(ActiveSaveGame) == true && ActiveSaveGame->HasSolvedPuzzle(PuzzleId) == true;
}

void URELocalSaveSubsystem::GetSolvedPuzzleIds(TArray<FName>& OutSolvedPuzzleIds) const
{
	OutSolvedPuzzleIds.Reset();
	if (IsValid(ActiveSaveGame) == true)
	{
		OutSolvedPuzzleIds = ActiveSaveGame->SolvedPuzzleIds;
	}
}

bool URELocalSaveSubsystem::SaveCheckpoint(FName CheckpointId, const FTransform& CheckpointTransform, bool bSaveImmediately)
{
	UREProgressSaveGame* SaveGame = EnsureActiveSaveGame();
	if (IsValid(SaveGame) == false || CheckpointId.IsNone() == true)
	{
		return false;
	}

	SaveGame->SetCheckpoint(CheckpointId, CheckpointTransform);
	OnSaveProgressChanged.Broadcast();
	return bSaveImmediately == false || PersistActiveSaveGame() == true;
}

bool URELocalSaveSubsystem::GetLastCheckpoint(FName& OutCheckpointId, FTransform& OutCheckpointTransform) const
{
	OutCheckpointId = NAME_None;
	OutCheckpointTransform = FTransform::Identity;

	if (IsValid(ActiveSaveGame) == false || ActiveSaveGame->LastCheckpoint.bHasValidTransform == false)
	{
		return false;
	}

	OutCheckpointId = ActiveSaveGame->LastCheckpoint.CheckpointId;
	OutCheckpointTransform = ActiveSaveGame->LastCheckpoint.Transform;
	return true;
}

bool URELocalSaveSubsystem::ClearActiveProgress(bool bSaveImmediately)
{
	UREProgressSaveGame* SaveGame = EnsureActiveSaveGame();
	if (IsValid(SaveGame) == false)
	{
		return false;
	}

	SaveGame->ClearProgress();
	OnSaveProgressChanged.Broadcast();
	return bSaveImmediately == false || PersistActiveSaveGame() == true;
}

UREProgressSaveGame* URELocalSaveSubsystem::GetActiveSaveGame() const
{
	return ActiveSaveGame;
}

UREProgressSaveGame* URELocalSaveSubsystem::CreateEmptySaveGame(int32 SlotIndex) const
{
	UREProgressSaveGame* NewSaveGame = Cast<UREProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UREProgressSaveGame::StaticClass()));
	if (IsValid(NewSaveGame) == true)
	{
		NewSaveGame->SlotIndex = FMath::Max(0, SlotIndex);
		NewSaveGame->SlotName = BuildSlotName(SlotIndex);
	}
	return NewSaveGame;
}

UREProgressSaveGame* URELocalSaveSubsystem::EnsureActiveSaveGame()
{
	if (IsValid(ActiveSaveGame) == false)
	{
		LoadSlot(ActiveSlotIndex);
	}
	return ActiveSaveGame;
}

bool URELocalSaveSubsystem::PersistActiveSaveGame()
{
	UREProgressSaveGame* SaveGame = EnsureActiveSaveGame();
	if (IsValid(SaveGame) == false)
	{
		return false;
	}

	SaveGame->SlotIndex = ActiveSlotIndex;
	SaveGame->SlotName = BuildSlotName(ActiveSlotIndex);
	SaveGame->TouchLastSavedAt();
	return UGameplayStatics::SaveGameToSlot(SaveGame, SaveGame->SlotName, UserIndex);
}
