#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Puzzles/Framework/REPuzzleTypes.h"
#include "REPuzzleManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREPuzzleSimpleSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREPuzzleStateChangedSignature, EREPuzzleState, NewState);

UCLASS(Abstract, Blueprintable)
class ROOMESCAPE_API AREPuzzleManager : public AActor
{
	GENERATED_BODY()

public:
	AREPuzzleManager();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FREPuzzleSimpleSignature OnPuzzleSolved;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FREPuzzleSimpleSignature OnPuzzleFailed;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FREPuzzleStateChangedSignature OnPuzzleStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle", meta = (AllowPrivateAccess = "true"))
	bool bStartActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Progress", meta = (AllowPrivateAccess = "true"))
	bool bSaveSolvedState = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Progress", meta = (AllowPrivateAccess = "true"))
	bool bAutoRestoreSavedSolvedState = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Progress", meta = (AllowPrivateAccess = "true"))
	FName ProgressId = NAME_None;

	UPROPERTY(ReplicatedUsing = OnRep_State, BlueprintReadOnly, Category = "Puzzle", meta = (AllowPrivateAccess = "true"))
	EREPuzzleState State = EREPuzzleState::Locked;

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ActivatePuzzle();

	UFUNCTION(Server, Reliable, Category = "Puzzle")
	void ServerActivatePuzzle();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Puzzle")
	void ResetToLocked();

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	EREPuzzleState GetPuzzleState() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle|Progress")
	FName GetProgressId() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle|Progress")
	bool ShouldSaveSolvedState() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsLocked() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsActive() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsSolved() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsFailed() const;

protected:
	UFUNCTION()
	virtual void OnRep_State();

	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
	void ReceivePuzzleStateChanged(EREPuzzleState NewState);

	virtual bool CanActivatePuzzle() const;
	virtual void HandlePuzzleActivated();
	virtual void HandlePuzzleLocked();
	virtual void HandlePuzzleSolved();
	virtual void HandlePuzzleFailed();
	virtual void HandleSavedSolvedStateRestored();

	void SetPuzzleState(EREPuzzleState NewState);
	void MarkSolved();
	void MarkFailed();

private:
	FName ResolveProgressId() const;
	bool TryRestoreSavedSolvedState();
	void SaveSolvedProgress() const;
};
