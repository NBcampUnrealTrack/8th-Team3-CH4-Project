#include "Puzzles/MirrorRoom/REMirrorRoomManager.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/MirrorRoom/REMirrorCue.h"
#include "Puzzles/MirrorRoom/REMirrorInputPanel.h"
#include "Puzzles/MirrorRoom/REMirrorPuzzleData.h"
#include "Puzzles/MirrorRoom/REMirrorRoomFunctionLibrary.h"

AREMirrorRoomManager::AREMirrorRoomManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AREMirrorRoomManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentStepIndex);
	DOREPLIFETIME(ThisClass, SubmittedInputIds);
	DOREPLIFETIME(ThisClass, SubmittedClockHours);
}

void AREMirrorRoomManager::SetMirrorPuzzleData(UREMirrorPuzzleData* InPuzzleData)
{
	if (HasAuthority() == false)
	{
		return;
	}

	PuzzleData = InPuzzleData;
	ResetRuntimeInput();
}

UREMirrorPuzzleData* AREMirrorRoomManager::GetMirrorPuzzleData() const
{
	return PuzzleData;
}

int32 AREMirrorRoomManager::GetCurrentStepIndex() const
{
	return CurrentStepIndex;
}

int32 AREMirrorRoomManager::GetTotalStepCount() const
{
	return IsValid(PuzzleData) == true ? PuzzleData->GetStepCount() : 0;
}

bool AREMirrorRoomManager::IsMirrorInputComplete() const
{
	return IsSolved() == true || CurrentStepIndex >= GetTotalStepCount();
}

FText AREMirrorRoomManager::GetSubmittedInputText() const
{
	return IsValid(PuzzleData) == true ? PuzzleData->BuildSubmittedInputText(SubmittedInputIds, SubmittedClockHours) : FText::GetEmpty();
}

void AREMirrorRoomManager::GetInputOptions(TArray<FREMirrorInputOption>& OutOptions) const
{
	OutOptions.Reset();
	if (IsValid(PuzzleData) == true)
	{
		PuzzleData->GetResolvedInputOptions(OutOptions);
	}
}

void AREMirrorRoomManager::RegisterCue(AREMirrorCue* Cue)
{
	if (IsValid(Cue) == false)
	{
		return;
	}

	RegisteredCues.AddUnique(Cue);
}

void AREMirrorRoomManager::RegisterInputPanel(AREMirrorInputPanel* InputPanel)
{
	if (IsValid(InputPanel) == false)
	{
		return;
	}

	RegisteredInputPanels.AddUnique(InputPanel);
}

bool AREMirrorRoomManager::SubmitInputId(AActor* SourceActor, AActor* Interactor, const FName& InputId, FREMirrorInputResult& OutResult)
{
	if (HasAuthority() == false || CanAcceptInput() == false || InputId.IsNone() == true)
	{
		FillResult(OutResult, false, false, false, WrongInputMessage);
		return false;
	}

	FREMirrorSolutionStep Step;
	if (PuzzleData->GetSolutionStep(CurrentStepIndex, Step) == false || CheckInputId(Step, InputId) == false)
	{
		if (bResetInputOnError == true)
		{
			ResetRuntimeInput();
		}
		if (bMarkFailedOnError == true)
		{
			MarkFailed();
		}
		BroadcastInputResult(SourceActor, false, WrongInputMessage);
		FillResult(OutResult, false, false, bResetInputOnError, WrongInputMessage);
		return false;
	}

	SubmittedInputIds.Add(InputId);
	const int32 NextStepIndex = CurrentStepIndex + 1;
	SetCurrentStepIndex(NextStepIndex);

	if (NextStepIndex >= GetTotalStepCount())
	{
		MarkSolved();
		BroadcastInputResult(SourceActor, true, SolvedMessage);
		FillResult(OutResult, true, true, false, SolvedMessage);
		return true;
	}

	BroadcastInputResult(SourceActor, true, CorrectInputMessage);
	FillResult(OutResult, true, false, false, CorrectInputMessage);
	return true;
}

bool AREMirrorRoomManager::SubmitClockHour(AActor* SourceActor, AActor* Interactor, int32 ClockHour, FREMirrorInputResult& OutResult)
{
	if (HasAuthority() == false || CanAcceptInput() == false)
	{
		FillResult(OutResult, false, false, false, WrongInputMessage);
		return false;
	}

	FREMirrorSolutionStep Step;
	if (PuzzleData->GetSolutionStep(CurrentStepIndex, Step) == false || CheckClockHour(Step, ClockHour) == false)
	{
		if (bResetInputOnError == true)
		{
			ResetRuntimeInput();
		}
		if (bMarkFailedOnError == true)
		{
			MarkFailed();
		}
		BroadcastInputResult(SourceActor, false, WrongInputMessage);
		FillResult(OutResult, false, false, bResetInputOnError, WrongInputMessage);
		return false;
	}

	SubmittedClockHours.Add(UREMirrorRoomFunctionLibrary::NormalizeClockHour(ClockHour));
	const int32 NextStepIndex = CurrentStepIndex + 1;
	SetCurrentStepIndex(NextStepIndex);

	if (NextStepIndex >= GetTotalStepCount())
	{
		MarkSolved();
		BroadcastInputResult(SourceActor, true, SolvedMessage);
		FillResult(OutResult, true, true, false, SolvedMessage);
		return true;
	}

	BroadcastInputResult(SourceActor, true, CorrectInputMessage);
	FillResult(OutResult, true, false, false, CorrectInputMessage);
	return true;
}

bool AREMirrorRoomManager::ResetInput(AActor* SourceActor, AActor* Interactor, FREMirrorInputResult& OutResult)
{
	if (HasAuthority() == false || IsActive() == false)
	{
		FillResult(OutResult, false, false, false, FText::GetEmpty());
		return false;
	}

	ResetRuntimeInput();
	FillResult(OutResult, true, false, true, FText::FromString(TEXT("입력이 초기화되었습니다.")));
	BroadcastInputResult(SourceActor, true, OutResult.ResultMessage);
	return true;
}

bool AREMirrorRoomManager::CanActivatePuzzle() const
{
	return Super::CanActivatePuzzle() == true && IsValid(PuzzleData) == true && PuzzleData->IsPuzzleDataValid() == true;
}

void AREMirrorRoomManager::HandlePuzzleActivated()
{
	ResetRuntimeInput();
}

void AREMirrorRoomManager::HandlePuzzleLocked()
{
	ResetRuntimeInput();
}

void AREMirrorRoomManager::HandlePuzzleSolved()
{
}

void AREMirrorRoomManager::HandlePuzzleFailed()
{
}

void AREMirrorRoomManager::OnRep_CurrentStepIndex()
{
	OnMirrorInputProgressChanged.Broadcast(CurrentStepIndex, GetTotalStepCount());
	ReceiveMirrorInputProgressChanged(CurrentStepIndex, GetTotalStepCount());
}

bool AREMirrorRoomManager::CanAcceptInput() const
{
	return IsActive() == true && IsValid(PuzzleData) == true && PuzzleData->IsPuzzleDataValid() == true && CurrentStepIndex < GetTotalStepCount();
}

void AREMirrorRoomManager::ResetRuntimeInput()
{
	SubmittedInputIds.Reset();
	SubmittedClockHours.Reset();
	SetCurrentStepIndex(0);
}

void AREMirrorRoomManager::SetCurrentStepIndex(int32 NewStepIndex)
{
	const int32 ClampedStepIndex = FMath::Clamp(NewStepIndex, 0, GetTotalStepCount());
	if (CurrentStepIndex == ClampedStepIndex)
	{
		return;
	}

	CurrentStepIndex = ClampedStepIndex;
	OnRep_CurrentStepIndex();
}

bool AREMirrorRoomManager::CheckInputId(const FREMirrorSolutionStep& Step, const FName& InputId) const
{
	return PuzzleData->InputType == EREMirrorInputType::SymbolSequence && Step.ExpectedInputId == InputId;
}

bool AREMirrorRoomManager::CheckClockHour(const FREMirrorSolutionStep& Step, int32 ClockHour) const
{
	return PuzzleData->InputType == EREMirrorInputType::ClockHourSequence && UREMirrorRoomFunctionLibrary::NormalizeClockHour(Step.ExpectedClockHour) == UREMirrorRoomFunctionLibrary::NormalizeClockHour(ClockHour);
}

void AREMirrorRoomManager::FillResult(FREMirrorInputResult& OutResult, bool bAccepted, bool bSolved, bool bReset, const FText& Message) const
{
	OutResult.bAccepted = bAccepted;
	OutResult.bSolved = bSolved;
	OutResult.bReset = bReset;
	OutResult.CurrentStepIndex = CurrentStepIndex;
	OutResult.ResultMessage = Message;
	OutResult.SubmittedInputText = GetSubmittedInputText();
}

void AREMirrorRoomManager::BroadcastInputResult(AActor* SourceActor, bool bAccepted, const FText& Message)
{
	OnMirrorInputResult.Broadcast(SourceActor, bAccepted, Message);
	ReceiveMirrorInputResult(SourceActor, bAccepted, Message);
}
