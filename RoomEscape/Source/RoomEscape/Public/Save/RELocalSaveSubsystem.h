#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RELocalSaveSubsystem.generated.h"

class UREProgressSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRELocalSaveSlotChangedSignature, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRELocalSaveProgressChangedSignature);

UCLASS()
class ROOMESCAPE_API URELocalSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FRELocalSaveSlotChangedSignature OnActiveSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FRELocalSaveProgressChangedSignature OnSaveLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FRELocalSaveProgressChangedSignature OnSaveProgressChanged;

public:
	UFUNCTION(BlueprintCallable, Category = "Save|Slot")
	bool SetActiveSlotIndex(int32 NewSlotIndex, bool bLoadImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Save|Slot")
	int32 GetActiveSlotIndex() const;

	UFUNCTION(BlueprintPure, Category = "Save|Slot")
	FString GetActiveSlotName() const;

	UFUNCTION(BlueprintPure, Category = "Save|Slot")
	FString BuildSlotName(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Save|Slot")
	bool DoesSlotExist(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Slot")
	void GetExistingSlotIndices(int32 MaxSlotCount, TArray<int32>& OutSlotIndices) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Slot")
	bool LoadSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save|Slot")
	bool SaveActiveSlot();

	UFUNCTION(BlueprintCallable, Category = "Save|Slot")
	bool DeleteSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool MarkPuzzleSolved(FName PuzzleId, bool bSaveImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool UnmarkPuzzleSolved(FName PuzzleId, bool bSaveImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Save|Progress")
	bool IsPuzzleSolved(FName PuzzleId) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	void GetSolvedPuzzleIds(TArray<FName>& OutSolvedPuzzleIds) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool SaveCheckpoint(FName CheckpointId, const FTransform& CheckpointTransform, bool bSaveImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Save|Progress")
	bool GetLastCheckpoint(FName& OutCheckpointId, FTransform& OutCheckpointTransform) const;

	UFUNCTION(BlueprintCallable, Category = "Save|Progress")
	bool ClearActiveProgress(bool bSaveImmediately = true);

	UFUNCTION(BlueprintPure, Category = "Save")
	UREProgressSaveGame* GetActiveSaveGame() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UREProgressSaveGame> ActiveSaveGame;

	int32 ActiveSlotIndex = 0;
	int32 UserIndex = 0;
	FString SlotNamePrefix = TEXT("RoomEscapeProgress");

private:
	UREProgressSaveGame* CreateEmptySaveGame(int32 SlotIndex) const;
	UREProgressSaveGame* EnsureActiveSaveGame();
	bool PersistActiveSaveGame();
};
