#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/REBombButton.h"
#include "Puzzles/BombDefusal/REBombDevice.h"
#include "Puzzles/BombDefusal/REBombPatternData.h"
#include "Puzzles/BombDefusal/REBombWire.h"
#include "Puzzles/Framework/REPuzzleResetPoint.h"
#include "UI/REBombFeedbackWidget.h"
#include "UI/REFadeWidget.h"

AREBombDefusalManager::AREBombDefusalManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AREBombDefusalManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true && IsActive() == true && RemainingTimeSeconds <= 0.0f)
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

float AREBombDefusalManager::GetBadEndingResetDelaySeconds() const
{
	return FMath::Max(0.0f, BadEndingFadeOutSeconds) + FMath::Max(0.0f, BadEndingBlackHoldSeconds);
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
		InButton->ApplyServerPressedState(false, nullptr);
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
		FailBomb(Wire, Interactor, FailureMessage);
		return false;
	}

	Wire->ApplyServerCutState(true);
	AdvanceStep(Wire, Interactor, BuildSuccessFeedbackMessage());
	return true;
}

bool AREBombDefusalManager::SubmitButtonToggle(AREBombButton* Button, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || IsValid(Button) == false)
	{
		return false;
	}

	const bool bNextPressed = Button->IsPressed() == false;

	FText FailureMessage;
	if (ValidateCurrentButtonStep(Button, bNextPressed, FailureMessage) == false)
	{
		FailBomb(Button, Interactor, FailureMessage);
		return false;
	}

	Button->ApplyServerPressedState(bNextPressed, ResolvePlayerState(Interactor));

	AdvanceStep(Button, Interactor, BuildSuccessFeedbackMessage());
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
	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	ResetRuntimeState(true);
}

void AREBombDefusalManager::HandlePuzzleSolved()
{
	StopTimer();
	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	if (IsValid(BombDevice) == true)
	{
		BombDevice->ApplyServerTime(RemainingTimeSeconds, GetTimeLimitSeconds());
		BombDevice->ApplyServerStep(CurrentStepIndex, GetTotalStepCount());
	}
}

void AREBombDefusalManager::HandlePuzzleFailed()
{
	StopTimer();
}

void AREBombDefusalManager::HandleSavedSolvedStateRestored()
{
	SetCurrentStepIndex(GetTotalStepCount());
	SetRemainingTimeSeconds(0.0f);
	HandlePuzzleSolved();
	OnPuzzleSolved.Broadcast();
}

void AREBombDefusalManager::OnRep_CurrentStepIndex()
{
	OnBombStepChanged.Broadcast(CurrentStepIndex, GetTotalStepCount());
}

void AREBombDefusalManager::OnRep_RemainingTimeSeconds()
{
	OnBombTimeChanged.Broadcast(RemainingTimeSeconds, GetTimeLimitSeconds());
}

void AREBombDefusalManager::MulticastBombInputResult_Implementation(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage)
{
	OnBombInputResult.Broadcast(SourceActor, bCorrect, ResultMessage);
	ReceiveBombInputResult(SourceActor, bCorrect, ResultMessage);
	ShowFeedbackWidgetLocal(SourceActor, TargetPlayerState, bCorrect, ResultMessage);
}

void AREBombDefusalManager::MulticastBombExploded_Implementation(AActor* SourceActor, const FText& ResultMessage)
{
	OnBombExploded.Broadcast(SourceActor, ResultMessage);
	ReceiveBombExploded(SourceActor, ResultMessage);
	PlayBadEndingFadeOutLocal();
}

void AREBombDefusalManager::MulticastBombRuntimeReset_Implementation()
{
	OnBombRuntimeReset.Broadcast();
	ReceiveBombRuntimeReset();
}

void AREBombDefusalManager::MulticastBombCheckpointRestored_Implementation()
{
	OnBombCheckpointRestored.Broadcast();
	ReceiveBombCheckpointRestored();
	PlayBadEndingFadeInLocal();
}

void AREBombDefusalManager::StartActiveRound()
{
	if (HasAuthority() == false || IsValid(PatternData) == false || PatternData->IsPatternValid() == false)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
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
			Button->ApplyServerPressedState(false, nullptr);
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
		FailBomb(this, nullptr, BuildFailureFeedbackMessage());
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

void AREBombDefusalManager::AdvanceStep(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage)
{
	APlayerState* TargetPlayerState = ResolvePlayerState(Interactor);
	MulticastBombInputResult(SourceActor, TargetPlayerState, true, ResultMessage);
	const int32 NextStepIndex = CurrentStepIndex + 1;
	if (NextStepIndex >= GetTotalStepCount())
	{
		CompleteBomb(SourceActor, Interactor);
		return;
	}

	SetCurrentStepIndex(NextStepIndex);
}

void AREBombDefusalManager::CompleteBomb(AActor* SourceActor, AActor* Interactor)
{
	SetCurrentStepIndex(GetTotalStepCount());
	MarkSolved();
	MulticastBombInputResult(SourceActor, ResolvePlayerState(Interactor), true, BuildSolvedFeedbackMessage());
}

void AREBombDefusalManager::FailBomb(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage)
{
	if (HasAuthority() == false || IsSolved() == true || IsFailed() == true)
	{
		return;
	}

	SetRemainingTimeSeconds(0.0f);
	const FText FailureFeedbackMessage = ResultMessage;
	MulticastBombInputResult(SourceActor, ResolvePlayerState(Interactor), false, FailureFeedbackMessage);
	MarkFailed();
	MulticastBombExploded(SourceActor, FailureFeedbackMessage);

	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	GetWorldTimerManager().SetTimer(BadEndingResetTimerHandle, this, &AREBombDefusalManager::ResolveBadEnding, GetBadEndingResetDelaySeconds(), false);
}

void AREBombDefusalManager::ResolveBadEnding()
{
	if (HasAuthority() == false || IsSolved() == true)
	{
		return;
	}

	TeleportPlayersToBadEndingCheckpoints();
	ResetRuntimeState(true);
	SetPuzzleState(EREPuzzleState::Active);
	StartTimer();
	MulticastBombRuntimeReset();
	MulticastBombCheckpointRestored();
}

void AREBombDefusalManager::TeleportPlayersToBadEndingCheckpoints()
{
	TArray<AREPuzzleResetPoint*> CheckpointPoints;
	ResolveBadEndingCheckpointPoints(CheckpointPoints);
	if (CheckpointPoints.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] Bad ending checkpoint is not configured. Manager=%s CheckpointId=%s"), *GetName(), *BadEndingCheckpointId.ToString());
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false)
		{
			continue;
		}

		APawn* Pawn = PlayerController->GetPawn();
		if (IsValid(Pawn) == false)
		{
			continue;
		}

		AREPuzzleResetPoint* ResetPoint = CheckpointPoints.IsValidIndex(PlayerIndex) == true ? CheckpointPoints[PlayerIndex] : CheckpointPoints[0];
		if (IsValid(ResetPoint) == false)
		{
			continue;
		}

		const FTransform CheckpointTransform = ResetPoint->GetCheckpointTransform();
		const FRotator CheckpointRotation = CheckpointTransform.GetRotation().Rotator();
		Pawn->TeleportTo(CheckpointTransform.GetLocation(), CheckpointRotation, false, true);
		PlayerController->SetControlRotation(CheckpointRotation);
		++PlayerIndex;
	}
}

void AREBombDefusalManager::ResolveBadEndingCheckpointPoints(TArray<AREPuzzleResetPoint*>& OutCheckpointPoints) const
{
	OutCheckpointPoints.Reset();

	for (const TObjectPtr<AREPuzzleResetPoint>& ResetPointPtr : BadEndingCheckpointPoints)
	{
		AREPuzzleResetPoint* ResetPoint = ResetPointPtr.Get();
		if (IsValid(ResetPoint) == true && ResetPoint->CanUseAsCheckpoint() == true)
		{
			OutCheckpointPoints.AddUnique(ResetPoint);
		}
	}

	if (OutCheckpointPoints.Num() <= 0 && BadEndingCheckpointId.IsNone() == false)
	{
		TArray<AActor*> ResetPointActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AREPuzzleResetPoint::StaticClass(), ResetPointActors);
		for (AActor* ResetPointActor : ResetPointActors)
		{
			AREPuzzleResetPoint* ResetPoint = Cast<AREPuzzleResetPoint>(ResetPointActor);
			if (IsValid(ResetPoint) == true && ResetPoint->CanUseAsCheckpoint() == true && ResetPoint->GetCheckpointId() == BadEndingCheckpointId)
			{
				OutCheckpointPoints.AddUnique(ResetPoint);
			}
		}
	}

	OutCheckpointPoints.Sort([](const AREPuzzleResetPoint& A, const AREPuzzleResetPoint& B)
	{
		if (A.GetCheckpointOrder() == B.GetCheckpointOrder())
		{
			return A.GetName() < B.GetName();
		}
		return A.GetCheckpointOrder() < B.GetCheckpointOrder();
	});
}

void AREBombDefusalManager::PlayBadEndingFadeOutLocal()
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(nullptr);
	if (IsValid(LocalPlayerController) == false || IsValid(BadEndingFadeWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false || ActiveBadEndingFadeWidget->GetClass() != BadEndingFadeWidgetClass)
	{
		ActiveBadEndingFadeWidget = CreateWidget<UREFadeWidget>(LocalPlayerController, BadEndingFadeWidgetClass);
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false)
	{
		return;
	}

	if (ActiveBadEndingFadeWidget->IsInViewport() == false)
	{
		ActiveBadEndingFadeWidget->AddToViewport(BadEndingFadeWidgetZOrder);
	}

	GetWorldTimerManager().ClearTimer(BadEndingFadeWidgetCleanupTimerHandle);
	ActiveBadEndingFadeWidget->PlayFadeOut(BadEndingFadeOutSeconds);
}

void AREBombDefusalManager::PlayBadEndingFadeInLocal()
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(nullptr);
	if (IsValid(LocalPlayerController) == false || IsValid(BadEndingFadeWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false || ActiveBadEndingFadeWidget->GetClass() != BadEndingFadeWidgetClass)
	{
		ActiveBadEndingFadeWidget = CreateWidget<UREFadeWidget>(LocalPlayerController, BadEndingFadeWidgetClass);
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false)
	{
		return;
	}

	if (ActiveBadEndingFadeWidget->IsInViewport() == false)
	{
		ActiveBadEndingFadeWidget->AddToViewport(BadEndingFadeWidgetZOrder);
	}

	ActiveBadEndingFadeWidget->PlayFadeIn(BadEndingFadeInSeconds);
	GetWorldTimerManager().ClearTimer(BadEndingFadeWidgetCleanupTimerHandle);
	GetWorldTimerManager().SetTimer(BadEndingFadeWidgetCleanupTimerHandle, this, &AREBombDefusalManager::CleanupBadEndingFadeWidgetLocal, FMath::Max(BadEndingFadeInSeconds, 0.0f) + 0.05f, false);
}

void AREBombDefusalManager::CleanupBadEndingFadeWidgetLocal()
{
	if (IsValid(ActiveBadEndingFadeWidget) == true)
	{
		ActiveBadEndingFadeWidget->RemoveFromParent();
		ActiveBadEndingFadeWidget = nullptr;
	}
}

void AREBombDefusalManager::ShowFeedbackWidgetLocal(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage)
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(TargetPlayerState);
	if (IsValid(LocalPlayerController) == false || IsValid(FeedbackWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveFeedbackWidget) == false || ActiveFeedbackWidget->GetClass() != FeedbackWidgetClass)
	{
		ActiveFeedbackWidget = CreateWidget<UREBombFeedbackWidget>(LocalPlayerController, FeedbackWidgetClass);
	}

	if (IsValid(ActiveFeedbackWidget) == false)
	{
		return;
	}

	if (ActiveFeedbackWidget->IsInViewport() == false)
	{
		ActiveFeedbackWidget->AddToViewport(80);
	}

	ActiveFeedbackWidget->InitializeFeedback(SourceActor, bCorrect, ResultMessage, FeedbackWidgetDisplaySeconds);
}

APlayerController* AREBombDefusalManager::ResolveLocalPlayerController(APlayerState* TargetPlayerState) const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
		{
			continue;
		}

		if (IsValid(TargetPlayerState) == false || PlayerController->PlayerState == TargetPlayerState)
		{
			return PlayerController;
		}
	}

	return nullptr;
}

FText AREBombDefusalManager::BuildSuccessFeedbackMessage() const
{
	return FText::FromString(TEXT("장치 조작이 입력되었습니다."));
}

FText AREBombDefusalManager::BuildSolvedFeedbackMessage() const
{
	return FText::FromString(TEXT("폭탄 해제가 완료되었습니다."));
}

FText AREBombDefusalManager::BuildFailureFeedbackMessage() const
{
	return FText::FromString(TEXT("폭탄이 폭발했습니다.\n체크포인트에서 다시 시작합니다."));
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
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	if (IsValid(Wire) == false || Wire->IsCut() == true || Wire->GetWireIndex() != CurrentStep.WireIndex)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	return true;
}

bool AREBombDefusalManager::ValidateCurrentButtonStep(const AREBombButton* Button, bool bNextPressed, FText& OutFailureMessage) const
{
	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false || CurrentStep.StepType != EREBombStepType::ButtonState)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	if (IsValid(Button) == false || Button->GetButtonId() != CurrentStep.ButtonId)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	if (bNextPressed != CurrentStep.bRequiredButtonPressed)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
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
