#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorRoomManager.generated.h"

class UREMirrorPuzzleData;
class AREMirrorCue;
class AREMirrorInputPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FREMirrorInputResultSignature, AActor*, SourceActor, bool, bAccepted, const FText&, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREMirrorInputProgressSignature, int32, CurrentStepIndex, int32, TotalStepCount);

UCLASS(Blueprintable)
class ROOMESCAPE_API AREMirrorRoomManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	AREMirrorRoomManager();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREMirrorPuzzleData> PuzzleData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room|Rule", meta = (AllowPrivateAccess = "true"))
	bool bResetInputOnError = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room|Rule", meta = (AllowPrivateAccess = "true"))
	bool bMarkFailedOnError = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room|Message", meta = (AllowPrivateAccess = "true"))
	FText CorrectInputMessage = FText::FromString(TEXT("입력이 등록되었습니다."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room|Message", meta = (AllowPrivateAccess = "true"))
	FText WrongInputMessage = FText::FromString(TEXT("입력이 초기화되었습니다."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Room|Message", meta = (AllowPrivateAccess = "true"))
	FText SolvedMessage = FText::FromString(TEXT("CLEAR\n입력이 완료되었습니다."));

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStepIndex, BlueprintReadOnly, Category = "Mirror Room", meta = (AllowPrivateAccess = "true"))
	int32 CurrentStepIndex = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Mirror Room", meta = (AllowPrivateAccess = "true"))
	TArray<FName> SubmittedInputIds;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Mirror Room", meta = (AllowPrivateAccess = "true"))
	TArray<int32> SubmittedClockHours;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AREMirrorCue>> RegisteredCues;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AREMirrorInputPanel>> RegisteredInputPanels;

public:
	UPROPERTY(BlueprintAssignable, Category = "Mirror Room")
	FREMirrorInputResultSignature OnMirrorInputResult;

	UPROPERTY(BlueprintAssignable, Category = "Mirror Room")
	FREMirrorInputProgressSignature OnMirrorInputProgressChanged;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mirror Room")
	void SetMirrorPuzzleData(UREMirrorPuzzleData* InPuzzleData);

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	UREMirrorPuzzleData* GetMirrorPuzzleData() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	int32 GetCurrentStepIndex() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	int32 GetTotalStepCount() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	bool IsMirrorInputComplete() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	FText GetSubmittedInputText() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	void GetInputOptions(TArray<FREMirrorInputOption>& OutOptions) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mirror Room")
	void RegisterCue(AREMirrorCue* Cue);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mirror Room")
	void RegisterInputPanel(AREMirrorInputPanel* InputPanel);

	bool SubmitInputId(AActor* SourceActor, AActor* Interactor, const FName& InputId, FREMirrorInputResult& OutResult);
	bool SubmitClockHour(AActor* SourceActor, AActor* Interactor, int32 ClockHour, FREMirrorInputResult& OutResult);
	bool ResetInput(AActor* SourceActor, AActor* Interactor, FREMirrorInputResult& OutResult);

protected:
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;
	virtual void HandlePuzzleFailed() override;

	UFUNCTION()
	void OnRep_CurrentStepIndex();

	UFUNCTION(BlueprintImplementableEvent, Category = "Mirror Room")
	void ReceiveMirrorInputResult(AActor* SourceActor, bool bAccepted, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mirror Room")
	void ReceiveMirrorInputProgressChanged(int32 NewStepIndex, int32 TotalStepCount);

private:
	bool CanAcceptInput() const;
	void ResetRuntimeInput();
	void SetCurrentStepIndex(int32 NewStepIndex);
	bool CheckInputId(const FREMirrorSolutionStep& Step, const FName& InputId) const;
	bool CheckClockHour(const FREMirrorSolutionStep& Step, int32 ClockHour) const;
	void FillResult(FREMirrorInputResult& OutResult, bool bAccepted, bool bSolved, bool bReset, const FText& Message) const;
	void BroadcastInputResult(AActor* SourceActor, bool bAccepted, const FText& Message);
};
