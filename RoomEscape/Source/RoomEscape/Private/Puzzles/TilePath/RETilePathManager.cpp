#include "Puzzles/TilePath/RETilePathManager.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/Framework/REPuzzleHintActor.h"
#include "Puzzles/TilePath/RETilePathTile.h"

ARETilePathManager::ARETilePathManager()
{
	PuzzleAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PuzzleAreaBox"));
	PuzzleAreaBox->SetupAttachment(SceneRoot);
	PuzzleAreaBox->SetBoxExtent(FVector(600.0, 600.0, 200.0));
	PuzzleAreaBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PuzzleAreaBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PuzzleAreaBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARETilePathManager::BeginPlay()
{
	Super::BeginPlay();

	PuzzleAreaBox->OnComponentBeginOverlap.AddDynamic(this, &ARETilePathManager::OnPuzzleAreaBeginOverlap);
	PuzzleAreaBox->OnComponentEndOverlap.AddDynamic(this, &ARETilePathManager::OnPuzzleAreaEndOverlap);

	if (HasAuthority() == true)
	{
		if (bSpawnTilesOnBeginPlay == true)
		{
			SpawnTiles();
		}

		SetRemainingTimeSeconds(bUseTimeLimit == true ? TimeLimitSeconds : 0.0f);

		if (IsValid(HintActor) == true)
		{
			HintActor->SetUnlocked(IsSolved());
		}

		RefreshTileStates();
	}
}

void ARETilePathManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentStepIndex);
	DOREPLIFETIME(ThisClass, RevealedMoveCount);
	DOREPLIFETIME(ThisClass, RemainingTimeSeconds);
	DOREPLIFETIME(ThisClass, WalkerPawn);
	DOREPLIFETIME(ThisClass, WalkerPlayerState);
	DOREPLIFETIME(ThisClass, GuidePlayerState);
	DOREPLIFETIME(ThisClass, bSessionStarted);
	DOREPLIFETIME(ThisClass, SessionPhase);
	DOREPLIFETIME(ThisClass, CountdownNumber);
}

bool ARETilePathManager::StartPuzzle(APawn* InWalkerPawn)
{
	if (HasAuthority() == false)
	{
		return false;
	}

	if (CanStartPuzzle(InWalkerPawn) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TilePath] Walker registration rejected. Pawn=%s Active=%d Started=%d Phase=%d SafePath=%d ExistingWalker=%s GuidePS=%s"), *GetNameSafe(InWalkerPawn), IsActive() ? 1 : 0, bSessionStarted ? 1 : 0, static_cast<int32>(SessionPhase), IsSafePathValid() ? 1 : 0, *GetNameSafe(WalkerPawn.Get()), *GetNameSafe(GuidePlayerState.Get()));
		return false;
	}

	WalkerPawn = InWalkerPawn;
	WalkerPlayerState = nullptr;

	if (AController* Controller = InWalkerPawn->GetController())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			WalkerPlayerState = PlayerController->PlayerState;
		}
	}

	SetCurrentStepIndex(0);
	SetRemainingTimeSeconds(bUseTimeLimit == true ? TimeLimitSeconds : 0.0f);
	BroadcastParticipantsChanged();
	RefreshTileStates();
	TryStartSession();
	UE_LOG(LogTemp, Warning, TEXT("[TilePath] Walker registered. Pawn=%s WalkerPS=%s GuidePS=%s Phase=%d"), *GetNameSafe(WalkerPawn.Get()), *GetNameSafe(WalkerPlayerState.Get()), *GetNameSafe(GuidePlayerState.Get()), static_cast<int32>(SessionPhase));
	return true;
}

bool ARETilePathManager::RegisterGuide(APlayerController* InGuideController)
{
	if (HasAuthority() == false || IsValid(InGuideController) == false || IsSolved() == true || IsLocked() == true || bSessionStarted == true)
	{
		return false;
	}

	if (IsValid(GuidePlayerState) == true && GuidePlayerState != InGuideController->PlayerState)
	{
		return false;
	}

	if (bAllowSamePlayerForBothRoles == false && IsValid(WalkerPlayerState) == true && WalkerPlayerState == InGuideController->PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TilePath] Guide registration rejected because same player is already Walker. Controller=%s"), *GetNameSafe(InGuideController));
		return false;
	}

	GuideController = InGuideController;
	GuidePlayerState = InGuideController->PlayerState;
	BroadcastParticipantsChanged();
	TryStartSession();
	UE_LOG(LogTemp, Warning, TEXT("[TilePath] Guide registered. Controller=%s WalkerPS=%s GuidePS=%s Phase=%d"), *GetNameSafe(InGuideController), *GetNameSafe(WalkerPlayerState.Get()), *GetNameSafe(GuidePlayerState.Get()), static_cast<int32>(SessionPhase));
	return true;
}

bool ARETilePathManager::CancelGuide(APlayerController* InGuideController)
{
	if (HasAuthority() == false || IsValid(InGuideController) == false || bSessionStarted == true)
	{
		return false;
	}

	if (IsValid(GuidePlayerState) == true && GuidePlayerState != InGuideController->PlayerState)
	{
		return false;
	}

	if (GuideController != nullptr && GuideController != InGuideController)
	{
		return false;
	}

	GuideController = nullptr;
	GuidePlayerState = nullptr;
	StopSessionCountdown();
	BroadcastParticipantsChanged();
	RefreshTileStates();
	return true;
}

bool ARETilePathManager::CancelWalker(APawn* InWalkerPawn)
{
	if (HasAuthority() == false || IsValid(InWalkerPawn) == false || bSessionStarted == true)
	{
		return false;
	}

	if (IsValid(WalkerPawn) == true && WalkerPawn != InWalkerPawn)
	{
		return false;
	}

	WalkerPawn = nullptr;
	WalkerPlayerState = nullptr;
	StopSessionCountdown();
	BroadcastParticipantsChanged();
	RefreshTileStates();
	return true;
}

bool ARETilePathManager::SubmitAnswer(APlayerController* RequestingController, FName QuestionId, ERETilePathAnswerChoice SubmittedAnswer, FRETilePathRevealedMove& OutRevealedMove)
{
	OutRevealedMove = FRETilePathRevealedMove();

	if (HasAuthority() == false || CanSubmitAnswer(RequestingController) == false)
	{
		return false;
	}

	if (RevealedMoveCount >= GetTotalMoveCount())
	{
		return false;
	}

	const FName ExpectedQuestionId = GetCurrentQuestionId();
	if (ExpectedQuestionId.IsNone() == true || ExpectedQuestionId != QuestionId)
	{
		return false;
	}

	FRETilePathQuestionRow QuestionRow;
	if (GetQuestionData(ExpectedQuestionId, QuestionRow) == false || QuestionRow.CorrectAnswer != SubmittedAnswer)
	{
		return false;
	}

	if (BuildRevealedMove(RevealedMoveCount, ExpectedQuestionId, OutRevealedMove) == false)
	{
		return false;
	}

	SetRevealedMoveCount(RevealedMoveCount + 1);
	RefreshTileStates();
	OnTilePathDirectionRevealed.Broadcast(OutRevealedMove);
	return true;
}

void ARETilePathManager::HandleTileStepped(ARETilePathTile* Tile, APawn* SteppingPawn)
{
	if (HasAuthority() == false || IsValid(Tile) == false || IsValid(SteppingPawn) == false)
	{
		return;
	}


	if (IsActive() == false || bSessionStarted == false || IsValid(WalkerPawn) == false || WalkerPawn != SteppingPawn)
	{
		return;
	}

	const FIntPoint TileCoordinate = Tile->GetTileCoordinate();

	if (SafePath.IsValidIndex(CurrentStepIndex) == true && TileCoordinate == SafePath[CurrentStepIndex])
	{
		MulticastHandleTileJudged(SteppingPawn, TileCoordinate, true);
		return;
	}

	const int32 NextStepIndex = CurrentStepIndex + 1;
	const bool bHasNextStep = SafePath.IsValidIndex(NextStepIndex) == true;
	const bool bIsNextSafeTile = bHasNextStep == true && TileCoordinate == SafePath[NextStepIndex];
	const bool bMoveWasRevealed = bRequireRevealedMoveToStep == false || NextStepIndex <= RevealedMoveCount;

	if (bIsNextSafeTile == true && bMoveWasRevealed == true)
	{
		SetCurrentStepIndex(NextStepIndex);
		RefreshTileStates();
		MulticastHandleTileJudged(SteppingPawn, TileCoordinate, true);

		if (NextStepIndex >= SafePath.Num() - 1)
		{
			ClearPuzzle();
		}
		return;
	}

	if (ARETilePathTile* FailedTile = FindTileByCoordinate(TileCoordinate))
	{
		FailedTile->ApplyServerTileState(ERETilePathTileState::Failed, FailedTile->IsWalkable());
	}

	UE_LOG(LogTemp, Warning, TEXT("[TilePath] Wrong tile stepped. Pawn=%s Tile=(%d,%d) CurrentStep=%d Revealed=%d"), *GetNameSafe(SteppingPawn), TileCoordinate.X, TileCoordinate.Y, CurrentStepIndex, RevealedMoveCount);
	MulticastHandleTileJudged(SteppingPawn, TileCoordinate, false);
	MulticastHandlePuzzleFailed(SteppingPawn, TileCoordinate);
}

void ARETilePathManager::RegisterTile(ARETilePathTile* Tile)
{
	if (IsValid(Tile) == false)
	{
		return;
	}

	TileActors.AddUnique(Tile);
	if (HasAuthority() == true)
	{
		RefreshTileStates();
	}
}

void ARETilePathManager::RefreshTileStates()
{
	if (HasAuthority() == false)
	{
		return;
	}

	for (ARETilePathTile* Tile : TileActors)
	{
		if (IsValid(Tile) == false)
		{
			continue;
		}

		const int32 PathIndex = FindSafePathIndex(Tile->GetTileCoordinate());
		bool bNewWalkable = bGateTileCollisionByRevealedPath == false;
		ERETilePathTileState NewTileState = ERETilePathTileState::Default;

		if (IsSolved() == true)
		{
			if (PathIndex != INDEX_NONE)
			{
				bNewWalkable = true;
				NewTileState = ERETilePathTileState::Solved;
			}
		}
		else if (IsActive() == true)
		{
			if (PathIndex != INDEX_NONE)
			{
				if (PathIndex < CurrentStepIndex)
				{
					bNewWalkable = true;
					NewTileState = ERETilePathTileState::Passed;
				}
				else if (PathIndex == CurrentStepIndex)
				{
					bNewWalkable = true;
					NewTileState = ERETilePathTileState::Current;
				}
				else if (PathIndex <= RevealedMoveCount)
				{
					bNewWalkable = true;
					NewTileState = ERETilePathTileState::Available;
				}
				else
				{
					NewTileState = ERETilePathTileState::Locked;
				}
			}
			else
			{
				NewTileState = ERETilePathTileState::Locked;
			}
		}
		else
		{
			NewTileState = ERETilePathTileState::Locked;
		}

		Tile->ApplyServerTileState(bRevealTileStateInWorld == true ? NewTileState : ERETilePathTileState::Default, bNewWalkable);
	}
}

bool ARETilePathManager::CanStartPuzzle(APawn* InWalkerPawn) const
{
	if (IsValid(InWalkerPawn) == false || IsActive() == false || bSessionStarted == true)
	{
		return false;
	}

	if (IsValid(WalkerPawn) == true && WalkerPawn != InWalkerPawn)
	{
		return false;
	}

	if (bAllowSamePlayerForBothRoles == false && IsValid(GuidePlayerState) == true)
	{
		if (AController* Controller = InWalkerPawn->GetController())
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
			{
				if (PlayerController->PlayerState == GuidePlayerState)
				{
					return false;
				}
			}
		}
	}

	if (IsSafePathValid() == false)
	{
		return false;
	}

	if (bRequireWalkerInsidePuzzleArea == true && IsPawnInsidePuzzleArea(InWalkerPawn) == false)
	{
		return false;
	}

	return true;
}

bool ARETilePathManager::CanSubmitAnswer(APlayerController* RequestingController) const
{
	if (IsValid(RequestingController) == false)
	{
		return false;
	}

	if (IsSolved() == true || IsLocked() == true || bSessionStarted == false)
	{
		return false;
	}

	if (bAllowGuideToSolveBeforeStart == false && IsValid(WalkerPawn) == false)
	{
		return false;
	}

	if (IsValid(GuideController) == true && GuideController != RequestingController)
	{
		return false;
	}

	if (GuidePlayerState != nullptr && GuidePlayerState != RequestingController->PlayerState)
	{
		return false;
	}

	if (IsSafePathValid() == false || RevealedMoveCount >= GetTotalMoveCount())
	{
		return false;
	}

	return true;
}

bool ARETilePathManager::IsSafePathValid() const
{
	if (GridSize < 2 || SafePath.Num() < 2)
	{
		return false;
	}

	for (int32 Index = 0; Index < SafePath.Num(); ++Index)
	{
		if (IsCoordinateInGrid(SafePath[Index]) == false)
		{
			return false;
		}

		if (Index > 0 && IsAdjacentStep(SafePath[Index - 1], SafePath[Index]) == false)
		{
			return false;
		}
	}

	return true;
}

bool ARETilePathManager::IsCoordinateInGrid(FIntPoint Coordinate) const
{
	return Coordinate.X >= 0 && Coordinate.Y >= 0 && Coordinate.X < GridSize && Coordinate.Y < GridSize;
}

int32 ARETilePathManager::GetTotalMoveCount() const
{
	return FMath::Max(SafePath.Num() - 1, 0);
}

bool ARETilePathManager::AreAllQuestionsSolved() const
{
	const int32 TotalMoveCount = GetTotalMoveCount();
	return TotalMoveCount > 0 && RevealedMoveCount >= TotalMoveCount;
}

int32 ARETilePathManager::GetCurrentStepIndex() const
{
	return CurrentStepIndex;
}

int32 ARETilePathManager::GetRevealedMoveCount() const
{
	return RevealedMoveCount;
}

float ARETilePathManager::GetRemainingTimeSeconds() const
{
	return RemainingTimeSeconds;
}

float ARETilePathManager::GetTimeLimitSeconds() const
{
	return TimeLimitSeconds;
}

bool ARETilePathManager::IsSessionStarted() const
{
	return bSessionStarted;
}

ERETilePathSessionPhase ARETilePathManager::GetSessionPhase() const
{
	return SessionPhase;
}

int32 ARETilePathManager::GetCountdownNumber() const
{
	return CountdownNumber;
}

bool ARETilePathManager::IsCountdownActive() const
{
	return SessionPhase == ERETilePathSessionPhase::Countdown;
}

bool ARETilePathManager::IsWalkerRegistered() const
{
	return IsValid(WalkerPawn);
}

bool ARETilePathManager::IsGuideRegistered() const
{
	return IsValid(GuidePlayerState);
}

APawn* ARETilePathManager::GetWalkerPawn() const
{
	return WalkerPawn;
}

APlayerState* ARETilePathManager::GetGuidePlayerState() const
{
	return GuidePlayerState;
}

void ARETilePathManager::GetSafePath(TArray<FIntPoint>& OutSafePath) const
{
	OutSafePath = SafePath;
}

void ARETilePathManager::GetRegisteredTiles(TArray<ARETilePathTile*>& OutTiles) const
{
	OutTiles.Reset();
	OutTiles.Reserve(TileActors.Num());

	for (ARETilePathTile* Tile : TileActors)
	{
		if (IsValid(Tile) == true)
		{
			OutTiles.Add(Tile);
		}
	}
}

ARETilePathTile* ARETilePathManager::FindTileByCoordinate(FIntPoint Coordinate) const
{
	for (ARETilePathTile* Tile : TileActors)
	{
		if (IsValid(Tile) == true && Tile->GetTileCoordinate() == Coordinate)
		{
			return Tile;
		}
	}

	return nullptr;
}

bool ARETilePathManager::GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const
{
	OutQuestionId = GetCurrentQuestionId();
	return GetQuestionData(OutQuestionId, OutQuestionRow);
}

bool ARETilePathManager::GetQuestionData(FName QuestionId, FRETilePathQuestionRow& OutQuestionRow) const
{
	if (QuestionId.IsNone() == true || IsValid(QuestionDataTable) == false || IsValid(QuestionDataTable->GetRowStruct()) == false)
	{
		return false;
	}

	if (QuestionDataTable->GetRowStruct()->IsChildOf(FRETilePathQuestionRow::StaticStruct()) == false)
	{
		return false;
	}

	const FRETilePathQuestionRow* QuestionRow = QuestionDataTable->FindRow<FRETilePathQuestionRow>(QuestionId, TEXT("TilePathQuestion"), false);
	if (QuestionRow == nullptr)
	{
		return false;
	}

	OutQuestionRow = *QuestionRow;
	return true;
}

bool ARETilePathManager::GetRevealedMoveByStep(int32 StepIndex, FRETilePathRevealedMove& OutRevealedMove) const
{
	if (StepIndex < 0 || StepIndex >= RevealedMoveCount || StepIndex >= GetTotalMoveCount())
	{
		OutRevealedMove = FRETilePathRevealedMove();
		return false;
	}

	const FName QuestionId = QuestionRowOrder.IsValidIndex(StepIndex) == true ? QuestionRowOrder[StepIndex] : NAME_None;
	return BuildRevealedMove(StepIndex, QuestionId, OutRevealedMove);
}

void ARETilePathManager::GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const
{
	OutRevealedMoves.Reset();
	OutRevealedMoves.Reserve(RevealedMoveCount);

	for (int32 Index = 0; Index < RevealedMoveCount; ++Index)
	{
		FRETilePathRevealedMove RevealedMove;
		if (GetRevealedMoveByStep(Index, RevealedMove) == true)
		{
			OutRevealedMoves.Add(RevealedMove);
		}
	}
}

FName ARETilePathManager::GetCurrentQuestionId() const
{
	if (QuestionRowOrder.IsValidIndex(RevealedMoveCount) == false)
	{
		return NAME_None;
	}

	return QuestionRowOrder[RevealedMoveCount];
}

void ARETilePathManager::OnRep_State()
{
	Super::OnRep_State();
	if (HasAuthority() == true)
	{
		RefreshTileStates();
	}
}

bool ARETilePathManager::CanActivatePuzzle() const
{
	return IsLocked() == true || IsFailed() == true;
}

void ARETilePathManager::HandlePuzzleActivated()
{
	StopSessionCountdown();
	SetSessionStarted(false);
	SetSessionPhase(ERETilePathSessionPhase::Waiting);
	SetCountdownNumber(-1);
	SetCurrentStepIndex(0);
	SetRemainingTimeSeconds(bUseTimeLimit == true ? TimeLimitSeconds : 0.0f);
	RefreshTileStates();
}

void ARETilePathManager::HandlePuzzleLocked()
{
	StopPuzzleTimer();
	StopSessionCountdown();
	WalkerPawn = nullptr;
	WalkerPlayerState = nullptr;
	GuideController = nullptr;
	GuidePlayerState = nullptr;
	SetSessionStarted(false);
	SetSessionPhase(ERETilePathSessionPhase::Waiting);
	SetCountdownNumber(-1);
	BroadcastParticipantsChanged();
	SetCurrentStepIndex(0);
	SetRevealedMoveCount(0);
	SetRemainingTimeSeconds(bUseTimeLimit == true ? TimeLimitSeconds : 0.0f);
	RefreshTileStates();
}

void ARETilePathManager::HandlePuzzleSolved()
{
	StopSessionCountdown();
	SetSessionStarted(false);
	SetSessionPhase(ERETilePathSessionPhase::Waiting);
	SetCountdownNumber(-1);
	RefreshTileStates();
}

void ARETilePathManager::HandlePuzzleFailed()
{
}

void ARETilePathManager::OnRep_CurrentStepIndex()
{
	OnTilePathProgressChanged.Broadcast(CurrentStepIndex, RevealedMoveCount);
}

void ARETilePathManager::OnRep_RevealedMoveCount()
{
	OnTilePathProgressChanged.Broadcast(CurrentStepIndex, RevealedMoveCount);
	OnTilePathQuestionChanged.Broadcast(GetCurrentQuestionId(), RevealedMoveCount);
}

void ARETilePathManager::OnRep_RemainingTimeSeconds()
{
	OnTilePathTimeChanged.Broadcast(RemainingTimeSeconds, TimeLimitSeconds);
}

void ARETilePathManager::OnRep_SessionStarted()
{
	OnTilePathSessionStateChanged.Broadcast(bSessionStarted);
}

void ARETilePathManager::OnRep_SessionPhase()
{
	OnTilePathSessionPhaseChanged.Broadcast(SessionPhase);
}

void ARETilePathManager::OnRep_CountdownNumber()
{
	OnTilePathCountdownChanged.Broadcast(CountdownNumber, CountdownNumber <= 0 && SessionPhase == ERETilePathSessionPhase::Countdown);
}

void ARETilePathManager::OnRep_ParticipantsChanged()
{
	OnTilePathParticipantsChanged.Broadcast();
}

void ARETilePathManager::OnPuzzleAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn) == true)
	{
		PawnsInsideArea.AddUnique(Pawn);
	}
}

void ARETilePathManager::OnPuzzleAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn) == true)
	{
		PawnsInsideArea.Remove(Pawn);
	}
}

void ARETilePathManager::MulticastHandleTileJudged_Implementation(APawn* Pawn, FIntPoint TileCoordinate, bool bCorrect)
{
	OnTilePathTileJudged.Broadcast(Pawn, TileCoordinate, bCorrect);
	ReceiveTileJudged(Pawn, TileCoordinate, bCorrect);
}

void ARETilePathManager::MulticastHandlePuzzleFailed_Implementation(APawn* Pawn, FIntPoint TileCoordinate)
{
	ReceivePuzzleFailed(Pawn, TileCoordinate);
}


void ARETilePathManager::MulticastHandlePuzzleCleared_Implementation(APawn* Pawn)
{
	OnTilePathCleared.Broadcast(Pawn);
	ReceivePuzzleCleared(Pawn);
}

void ARETilePathManager::SpawnTiles()
{
	if (HasAuthority() == false || IsValid(TileClass) == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	for (int32 PathIndex = 0; PathIndex < GridSize; ++PathIndex)
	{
		for (int32 ColumnIndex = 0; ColumnIndex < GridSize; ++ColumnIndex)
		{
			const FIntPoint TileCoordinate(PathIndex, ColumnIndex);
			const FVector LocalOffset(PathIndex * TileSpacing, -ColumnIndex * TileSpacing, 0.0);
			const FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(LocalOffset);
			const FTransform SpawnTransform(GetActorRotation(), SpawnLocation, FVector::OneVector);
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ARETilePathTile* Tile = World->SpawnActor<ARETilePathTile>(TileClass, SpawnTransform, SpawnParameters);
			if (IsValid(Tile) == true)
			{
				Tile->SetTileCoordinate(TileCoordinate);
				Tile->SetTilePathManager(this);
			}
		}
	}

	RefreshTileStates();
}

void ARETilePathManager::SetCurrentStepIndex(int32 NewCurrentStepIndex)
{
	const int32 ClampedStepIndex = FMath::Clamp(NewCurrentStepIndex, 0, FMath::Max(SafePath.Num() - 1, 0));
	if (CurrentStepIndex == ClampedStepIndex)
	{
		return;
	}

	CurrentStepIndex = ClampedStepIndex;
	OnRep_CurrentStepIndex();
}

void ARETilePathManager::SetRevealedMoveCount(int32 NewRevealedMoveCount)
{
	const int32 ClampedMoveCount = FMath::Clamp(NewRevealedMoveCount, 0, GetTotalMoveCount());
	if (RevealedMoveCount == ClampedMoveCount)
	{
		return;
	}

	RevealedMoveCount = ClampedMoveCount;
	OnRep_RevealedMoveCount();
}

void ARETilePathManager::SetRemainingTimeSeconds(float NewRemainingTimeSeconds)
{
	const float ClampedRemainingTimeSeconds = FMath::Max(NewRemainingTimeSeconds, 0.0f);
	if (FMath::IsNearlyEqual(RemainingTimeSeconds, ClampedRemainingTimeSeconds) == true)
	{
		return;
	}

	RemainingTimeSeconds = ClampedRemainingTimeSeconds;
	OnRep_RemainingTimeSeconds();
}

void ARETilePathManager::TryStartSession()
{
	if (HasAuthority() == false || bSessionStarted == true || SessionPhase == ERETilePathSessionPhase::Countdown || IsActive() == false || IsValid(WalkerPawn) == false || IsValid(GuidePlayerState) == false || IsSafePathValid() == false)
	{
		return;
	}

	StartSessionCountdown();
}

void ARETilePathManager::StartSessionCountdown()
{
	if (HasAuthority() == false || bSessionStarted == true || SessionPhase == ERETilePathSessionPhase::Countdown)
	{
		return;
	}

	SetCurrentStepIndex(0);
	SetRemainingTimeSeconds(bUseTimeLimit == true ? TimeLimitSeconds : 0.0f);
	SetSessionPhase(ERETilePathSessionPhase::Countdown);
	SetCountdownNumber(FMath::Max(CountdownStartNumber, 1));
	RefreshTileStates();

	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorldTimerManager().ClearTimer(GameStartTimerHandle);
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ARETilePathManager::AdvanceSessionCountdown, FMath::Max(CountdownIntervalSeconds, 0.1f), true);
}

void ARETilePathManager::AdvanceSessionCountdown()
{
	if (HasAuthority() == false || SessionPhase != ERETilePathSessionPhase::Countdown || bSessionStarted == true)
	{
		return;
	}

	if (IsValid(WalkerPawn) == false || IsValid(GuidePlayerState) == false || IsActive() == false)
	{
		StopSessionCountdown();
		BroadcastParticipantsChanged();
		return;
	}

	if (CountdownNumber > 1)
	{
		SetCountdownNumber(CountdownNumber - 1);
		return;
	}

	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	SetCountdownNumber(0);
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ARETilePathManager::FinishSessionCountdown, FMath::Max(GameStartDisplaySeconds, 0.1f), false);
}

void ARETilePathManager::FinishSessionCountdown()
{
	if (HasAuthority() == false || SessionPhase != ERETilePathSessionPhase::Countdown || bSessionStarted == true)
	{
		return;
	}

	if (IsValid(WalkerPawn) == false || IsValid(GuidePlayerState) == false || IsActive() == false || IsSafePathValid() == false)
	{
		StopSessionCountdown();
		BroadcastParticipantsChanged();
		return;
	}

	SetSessionStarted(true);
	SetSessionPhase(ERETilePathSessionPhase::Playing);
	SetCountdownNumber(-1);
	RefreshTileStates();
	StartPuzzleTimer();
}

void ARETilePathManager::StopSessionCountdown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		World->GetTimerManager().ClearTimer(GameStartTimerHandle);
	}

	if (HasAuthority() == true && bSessionStarted == false)
	{
		SetSessionPhase(ERETilePathSessionPhase::Waiting);
		SetCountdownNumber(-1);
	}
}

void ARETilePathManager::SetSessionStarted(bool bNewSessionStarted)
{
	if (bSessionStarted == bNewSessionStarted)
	{
		return;
	}

	bSessionStarted = bNewSessionStarted;
	OnRep_SessionStarted();
}

void ARETilePathManager::SetSessionPhase(ERETilePathSessionPhase NewSessionPhase)
{
	if (SessionPhase == NewSessionPhase)
	{
		return;
	}

	SessionPhase = NewSessionPhase;
	OnRep_SessionPhase();
}

void ARETilePathManager::SetCountdownNumber(int32 NewCountdownNumber)
{
	if (CountdownNumber == NewCountdownNumber)
	{
		return;
	}

	CountdownNumber = NewCountdownNumber;
	OnRep_CountdownNumber();
}

void ARETilePathManager::BroadcastParticipantsChanged()
{
	OnRep_ParticipantsChanged();
}

void ARETilePathManager::StartPuzzleTimer()
{
	if (HasAuthority() == false || bUseTimeLimit == false)
	{
		return;
	}

	StopPuzzleTimer();
	GetWorldTimerManager().SetTimer(PuzzleTimerHandle, this, &ARETilePathManager::OnPuzzleTimerTick, 1.0f, true);
}

void ARETilePathManager::StopPuzzleTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PuzzleTimerHandle);
	}
}

void ARETilePathManager::OnPuzzleTimerTick()
{
	if (HasAuthority() == false || IsActive() == false || bSessionStarted == false || IsValid(WalkerPawn) == false)
	{
		return;
	}

	SetRemainingTimeSeconds(RemainingTimeSeconds - 1.0f);

	if (RemainingTimeSeconds <= 0.0f)
	{
		StopPuzzleTimer();
		MarkFailed();
		MulticastHandlePuzzleFailed(WalkerPawn, SafePath.IsValidIndex(CurrentStepIndex) == true ? SafePath[CurrentStepIndex] : FIntPoint::ZeroValue);
	}
}

void ARETilePathManager::ClearPuzzle()
{
	if (HasAuthority() == false || IsSolved() == true)
	{
		return;
	}

	StopPuzzleTimer();
	MarkSolved();

	if (IsValid(HintActor) == true)
	{
		HintActor->SetUnlocked(true);
	}

	RefreshTileStates();
	MulticastHandlePuzzleCleared(WalkerPawn);
}


bool ARETilePathManager::IsPawnInsidePuzzleArea(APawn* Pawn) const
{
	return IsValid(Pawn) == true && PawnsInsideArea.Contains(Pawn) == true;
}

bool ARETilePathManager::IsAdjacentStep(FIntPoint From, FIntPoint To) const
{
	return FMath::Abs(From.X - To.X) + FMath::Abs(From.Y - To.Y) == 1;
}

bool ARETilePathManager::BuildRevealedMove(int32 StepIndex, FName QuestionId, FRETilePathRevealedMove& OutRevealedMove) const
{
	if (SafePath.IsValidIndex(StepIndex) == false || SafePath.IsValidIndex(StepIndex + 1) == false)
	{
		OutRevealedMove = FRETilePathRevealedMove();
		return false;
	}

	OutRevealedMove.StepIndex = StepIndex;
	OutRevealedMove.QuestionId = QuestionId;
	OutRevealedMove.FromCoordinate = SafePath[StepIndex];
	OutRevealedMove.ToCoordinate = SafePath[StepIndex + 1];
	OutRevealedMove.Direction = CalculateDirection(OutRevealedMove.FromCoordinate, OutRevealedMove.ToCoordinate);
	return OutRevealedMove.Direction != ERETilePathDirection::None;
}

ERETilePathDirection ARETilePathManager::CalculateDirection(FIntPoint From, FIntPoint To) const
{
	const FIntPoint Delta = To - From;
	if (Delta == FIntPoint(1, 0))
	{
		return ERETilePathDirection::Up;
	}

	if (Delta == FIntPoint(-1, 0))
	{
		return ERETilePathDirection::Down;
	}

	if (Delta == FIntPoint(0, 1))
	{
		return ERETilePathDirection::Left;
	}

	if (Delta == FIntPoint(0, -1))
	{
		return ERETilePathDirection::Right;
	}

	return ERETilePathDirection::None;
}


int32 ARETilePathManager::FindSafePathIndex(FIntPoint Coordinate) const
{
	for (int32 Index = 0; Index < SafePath.Num(); ++Index)
	{
		if (SafePath[Index] == Coordinate)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}
