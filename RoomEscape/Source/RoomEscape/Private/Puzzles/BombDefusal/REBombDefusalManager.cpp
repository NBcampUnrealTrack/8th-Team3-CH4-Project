#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/REBombButton.h"
#include "Puzzles/BombDefusal/REBombDevice.h"
#include "Puzzles/BombDefusal/REBombPatternData.h"
#include "Puzzles/BombDefusal/REBombWire.h"

AREBombDefusalManager::AREBombDefusalManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AREBombDefusalManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true && IsActive() == true)
	{
		StartActiveRound();
	}
}

void AREBombDefusalManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentStepIndex);
	DOREPLIFETIME(ThisClass, RemainingTimeSeconds);
}

void AREBombDefusalManager::SetPatternData(UREBombPatternData* InPatternData)
{
	if (HasAuthority() == false)
	{
		return;
	}

	PatternData = InPatternData;
	if (IsActive() == true)
	{
		StartActiveRound();
	}
}

UREBombPatternData* AREBombDefusalManager::GetPatternData() const
{
	return PatternData;
}

int32 AREBombDefusalManager::GetCurrentStepIndex() const
{
	return CurrentStepIndex;
}

int32 AREBombDefusalManager::GetTotalStepCount() const
{
	return IsValid(PatternData) == true ? PatternData->GetStepCount() : 0;
}

float AREBombDefusalManager::GetRemainingTimeSeconds() const
{
	return RemainingTimeSeconds;
}

float AREBombDefusalManager::GetTimeLimitSeconds() const
{
	return IsValid(PatternData) == true ? FMath::Max(PatternData->TimeLimitSeconds, 0.0f) : 0.0f;
}

bool AREBombDefusalManager::IsBombRunning() const
{
	return IsActive() == true && RemainingTimeSeconds > 0.0f && IsValid(PatternData) == true;
}

bool AREBombDefusalManager::GetCurrentStep(FREBombStep& OutStep) const
{
	return IsValid(PatternData) == true && PatternData->GetSolutionStep(CurrentStepIndex, OutStep) == true;
}

void AREBombDefusalManager::RegisterDevice(AREBombDevice* InDevice)
{
	if (IsValid(InDevice) == false)
	{
		return;
	}

	BombDevice = InDevice;
	if (HasAuthority() == true)
	{
		BombDevice->ApplyServerTime(RemainingTimeSeconds, GetTimeLimitSeconds());
		BombDevice->ApplyServerStep(CurrentStepIndex, GetTotalStepCount());
	}
}

void AREBombDefusalManager::RegisterWire(AREBombWire* InWire)
{
	if (IsValid(InWire) == false)
	{
		return;
	}

	Wires.AddUnique(InWire);
	if (HasAuthority() == true)
	{
		ApplyPatternToRegisteredActors();
	}
}

void AREBombDefusalManager::RegisterButton(AREBombButton* InButton)
{
	if (IsValid(InButton) == false)
	{
		return;
	}

	Buttons.AddUnique(InButton);
	if (HasAuthority() == true)
	{
		InButton->ApplyServerPressedState(false, nullptr, 0.0f);
	}
}

bool AREBombDefusalManager::SubmitWireCut(AREBombWire* Wire, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || IsValid(Wire) == false)
	{
		return false;
	}

	FText FailureMessage;
	if (ValidateCurrentWireStep(Wire, FailureMessage) == false)
	{
		FailBomb(Wire, FailureMessage);
		return false;
	}

	Wire->ApplyServerCutState(true);
	AdvanceStep(Wire, FText::FromString(TEXT("선 절단 완료")));
	return true;
}

bool AREBombDefusalManager::SubmitButtonPress(AREBombButton* Button, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || IsValid(Button) == false)
	{
		return false;
	}

	FText FailureMessage;
	if (ValidateCurrentButtonStep(Button, FailureMessage) == false)
	{
		FailBomb(Button, FailureMessage);
		return false;
	}

	if (Button->IsPressed() == true)
	{
		return false;
	}

	Button->ApplyServerPressedState(true, ResolvePlayerState(Interactor), GetWorld()->GetTimeSeconds());
	MulticastBombInputResult(Button, true, FText::FromString(TEXT("버튼 홀드 시작")));
	return true;
}

bool AREBombDefusalManager::SubmitButtonRelease(AREBombButton* Button, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || IsValid(Button) == false)
	{
		return false;
	}

	FText FailureMessage;
	if (ValidateCurrentButtonStep(Button, FailureMessage) == false)
	{
		FailBomb(Button, FailureMessage);
		return false;
	}

	if (Button->IsPressed() == false)
	{
		return false;
	}

	APlayerState* InteractorPlayerState = ResolvePlayerState(Interactor);
	if (IsValid(Button->GetPressingPlayerState()) == true && IsValid(InteractorPlayerState) == true && Button->GetPressingPlayerState() != InteractorPlayerState)
	{
		return false;
	}

	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false)
	{
		return false;
	}

	const float HoldDuration = FMath::Max(GetWorld()->GetTimeSeconds() - Button->GetPressStartServerTimeSeconds(), 0.0f);
	const bool bHoldAccepted = FMath::Abs(HoldDuration - CurrentStep.RequiredHoldSeconds) <= CurrentStep.HoldTolerance;
	Button->ApplyServerPressedState(false, nullptr, 0.0f);

	if (bHoldAccepted == false)
	{
		FailBomb(Button, FText::Format(FText::FromString(TEXT("홀드 시간 불일치: {0}초")), FText::AsNumber(HoldDuration)));
		return false;
	}

	AdvanceStep(Button, FText::FromString(TEXT("버튼 홀드 완료")));
	return true;
}

bool AREBombDefusalManager::CanActivatePuzzle() const
{
	return Super::CanActivatePuzzle() == true && IsValid(PatternData) == true && PatternData->IsPatternValid() == true;
}

void AREBombDefusalManager::HandlePuzzleActivated()
{
	StartActiveRound();
}

void AREBombDefusalManager::HandlePuzzleLocked()
{
	StopTimer();
	ResetRuntimeState(true);
}

void AREBombDefusalManager::HandlePuzzleSolved()
{
	StopTimer();
	if (IsValid(BombDevice) == true)
	{
		BombDevice->ApplyServerTime(RemainingTimeSeconds, GetTimeLimitSeconds());
		BombDevice->ApplyServerStep(CurrentStepIndex, GetTotalStepCount());
	}
}

void AREBombDefusalManager::HandlePuzzleFailed()
{
	StopTimer();
	if (bAutoResetOnFailure == true)
	{
		GetWorldTimerManager().ClearTimer(FailureResetTimerHandle);
		GetWorldTimerManager().SetTimer(FailureResetTimerHandle, this, &AREBombDefusalManager::ResetAfterFailure, ResetDelaySeconds, false);
	}
}

void AREBombDefusalManager::OnRep_CurrentStepIndex()
{
	OnBombStepChanged.Broadcast(CurrentStepIndex, GetTotalStepCount());
}

void AREBombDefusalManager::OnRep_RemainingTimeSeconds()
{
	OnBombTimeChanged.Broadcast(RemainingTimeSeconds, GetTimeLimitSeconds());
}

void AREBombDefusalManager::MulticastBombInputResult_Implementation(AActor* SourceActor, bool bCorrect, const FText& ResultMessage)
{
	OnBombInputResult.Broadcast(SourceActor, bCorrect, ResultMessage);
	ReceiveBombInputResult(SourceActor, bCorrect, ResultMessage);
}

void AREBombDefusalManager::MulticastBombRuntimeReset_Implementation()
{
	OnBombRuntimeReset.Broadcast();
	ReceiveBombRuntimeReset();
}

void AREBombDefusalManager::StartActiveRound()
{
	if (HasAuthority() == false || IsValid(PatternData) == false || PatternData->IsPatternValid() == false)
	{
		return;
	}

	ResetRuntimeState(true);
	StartTimer();
}

void AREBombDefusalManager::ResetRuntimeState(bool bResetDeviceState)
{
	SetCurrentStepIndex(0);
	SetRemainingTimeSeconds(GetTimeLimitSeconds());

	if (bResetDeviceState == true)
	{
		ApplyPatternToRegisteredActors();
	}

	if (IsValid(BombDevice) == true)
	{
		BombDevice->ApplyServerTime(RemainingTimeSeconds, GetTimeLimitSeconds());
		BombDevice->ApplyServerStep(CurrentStepIndex, GetTotalStepCount());
	}
}

void AREBombDefusalManager::ApplyPatternToRegisteredActors()
{
	if (HasAuthority() == false)
	{
		return;
	}

	for (AREBombWire* Wire : Wires)
	{
		if (IsValid(Wire) == false)
		{
			continue;
		}

		FREBombWireDefinition WireDefinition;
		if (IsValid(PatternData) == true && PatternData->GetWireDefinition(Wire->GetWireIndex(), WireDefinition) == true)
		{
			Wire->ApplyServerWireDefinition(WireDefinition);
		}
		Wire->ApplyServerCutState(false);
	}

	for (AREBombButton* Button : Buttons)
	{
		if (IsValid(Button) == true)
		{
			Button->ApplyServerPressedState(false, nullptr, 0.0f);
		}
	}
}

void AREBombDefusalManager::StartTimer()
{
	StopTimer();
	SetRemainingTimeSeconds(GetTimeLimitSeconds());
	GetWorldTimerManager().SetTimer(BombTimerHandle, this, &AREBombDefusalManager::TickTimer, 0.1f, true);
}

void AREBombDefusalManager::StopTimer()
{
	GetWorldTimerManager().ClearTimer(BombTimerHandle);
}

void AREBombDefusalManager::TickTimer()
{
	if (HasAuthority() == false || IsActive() == false)
	{
		StopTimer();
		return;
	}

	const float NewRemainingTime = FMath::Max(RemainingTimeSeconds - 0.1f, 0.0f);
	SetRemainingTimeSeconds(NewRemainingTime);
	if (NewRemainingTime <= 0.0f)
	{
		FailBomb(this, FText::FromString(TEXT("시간 초과")));
	}
}

void AREBombDefusalManager::SetCurrentStepIndex(int32 NewStepIndex)
{
	const int32 ClampedStepIndex = FMath::Clamp(NewStepIndex, 0, GetTotalStepCount());
	if (CurrentStepIndex == ClampedStepIndex)
	{
		return;
	}

	CurrentStepIndex = ClampedStepIndex;
	OnRep_CurrentStepIndex();
	if (IsValid(BombDevice) == true)
	{
		BombDevice->ApplyServerStep(CurrentStepIndex, GetTotalStepCount());
	}
}

void AREBombDefusalManager::SetRemainingTimeSeconds(float NewRemainingTimeSeconds)
{
	const float ClampedRemainingTime = FMath::Max(NewRemainingTimeSeconds, 0.0f);
	if (FMath::IsNearlyEqual(RemainingTimeSeconds, ClampedRemainingTime, 0.01f) == true)
	{
		return;
	}

	RemainingTimeSeconds = ClampedRemainingTime;
	OnRep_RemainingTimeSeconds();
	if (IsValid(BombDevice) == true)
	{
		BombDevice->ApplyServerTime(RemainingTimeSeconds, GetTimeLimitSeconds());
	}
}

void AREBombDefusalManager::AdvanceStep(AActor* SourceActor, const FText& ResultMessage)
{
	MulticastBombInputResult(SourceActor, true, ResultMessage);
	const int32 NextStepIndex = CurrentStepIndex + 1;
	if (NextStepIndex >= GetTotalStepCount())
	{
		CompleteBomb(SourceActor);
		return;
	}

	SetCurrentStepIndex(NextStepIndex);
}

void AREBombDefusalManager::CompleteBomb(AActor* SourceActor)
{
	SetCurrentStepIndex(GetTotalStepCount());
	MarkSolved();
	MulticastBombInputResult(SourceActor, true, FText::FromString(TEXT("폭탄 해제 완료")));
}

void AREBombDefusalManager::FailBomb(AActor* SourceActor, const FText& ResultMessage)
{
	if (HasAuthority() == false || IsSolved() == true || IsFailed() == true)
	{
		return;
	}

	MulticastBombInputResult(SourceActor, false, ResultMessage);
	MarkFailed();
}

void AREBombDefusalManager::ResetAfterFailure()
{
	if (HasAuthority() == false || IsSolved() == true)
	{
		return;
	}

	ResetRuntimeState(true);
	SetPuzzleState(EREPuzzleState::Active);
	StartTimer();
	MulticastBombRuntimeReset();
}

bool AREBombDefusalManager::CanAcceptInput() const
{
	return IsActive() == true && IsValid(PatternData) == true && PatternData->IsPatternValid() == true && CurrentStepIndex < GetTotalStepCount();
}

bool AREBombDefusalManager::ValidateCurrentWireStep(const AREBombWire* Wire, FText& OutFailureMessage) const
{
	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false || CurrentStep.StepType != EREBombStepType::CutWire)
	{
		OutFailureMessage = FText::FromString(TEXT("현재 단계는 선 절단 단계가 아닙니다"));
		return false;
	}

	if (IsValid(Wire) == false || Wire->IsCut() == true || Wire->GetWireIndex() != CurrentStep.WireIndex)
	{
		OutFailureMessage = FText::FromString(TEXT("잘못된 선을 절단했습니다"));
		return false;
	}

	return true;
}

bool AREBombDefusalManager::ValidateCurrentButtonStep(const AREBombButton* Button, FText& OutFailureMessage) const
{
	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false || CurrentStep.StepType != EREBombStepType::HoldButton)
	{
		OutFailureMessage = FText::FromString(TEXT("현재 단계는 버튼 홀드 단계가 아닙니다"));
		return false;
	}

	if (IsValid(Button) == false || Button->GetButtonId() != CurrentStep.ButtonId)
	{
		OutFailureMessage = FText::FromString(TEXT("잘못된 버튼을 조작했습니다"));
		return false;
	}

	return true;
}

APlayerState* AREBombDefusalManager::ResolvePlayerState(AActor* Actor) const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		return PlayerController->PlayerState;
	}

	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn->GetPlayerState();
	}

	return nullptr;
}
