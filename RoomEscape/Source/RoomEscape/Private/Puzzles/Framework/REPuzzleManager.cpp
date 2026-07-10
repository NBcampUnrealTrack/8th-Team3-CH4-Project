#include "Puzzles/Framework/REPuzzleManager.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Save/RELocalSaveSubsystem.h"

AREPuzzleManager::AREPuzzleManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AREPuzzleManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		if (TryRestoreSavedSolvedState() == true)
		{
			return;
		}

		if (bStartActive == true && State == EREPuzzleState::Locked)
		{
			ActivatePuzzle();
		}
	}
}

void AREPuzzleManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, State);
}

void AREPuzzleManager::ActivatePuzzle()
{
	if (HasAuthority() == false)
	{
		ServerActivatePuzzle();
		return;
	}

	if (CanActivatePuzzle() == false)
	{
		return;
	}

	SetPuzzleState(EREPuzzleState::Active);
	HandlePuzzleActivated();
}

void AREPuzzleManager::ServerActivatePuzzle_Implementation()
{
	ActivatePuzzle();
}

void AREPuzzleManager::ResetToLocked()
{
	if (HasAuthority() == false)
	{
		return;
	}

	SetPuzzleState(EREPuzzleState::Locked);
	HandlePuzzleLocked();
}

EREPuzzleState AREPuzzleManager::GetPuzzleState() const
{
	return State;
}

FName AREPuzzleManager::GetProgressId() const
{
	return ResolveProgressId();
}

bool AREPuzzleManager::ShouldSaveSolvedState() const
{
	return bSaveSolvedState == true && ResolveProgressId().IsNone() == false;
}

bool AREPuzzleManager::IsLocked() const
{
	return State == EREPuzzleState::Locked;
}

bool AREPuzzleManager::IsActive() const
{
	return State == EREPuzzleState::Active;
}

bool AREPuzzleManager::IsSolved() const
{
	return State == EREPuzzleState::Solved;
}

bool AREPuzzleManager::IsFailed() const
{
	return State == EREPuzzleState::Failed;
}

void AREPuzzleManager::OnRep_State()
{
	OnPuzzleStateChanged.Broadcast(State);
	ReceivePuzzleStateChanged(State);
}

bool AREPuzzleManager::CanActivatePuzzle() const
{
	return State == EREPuzzleState::Locked || State == EREPuzzleState::Failed;
}

void AREPuzzleManager::HandlePuzzleActivated()
{
}

void AREPuzzleManager::HandlePuzzleLocked()
{
}

void AREPuzzleManager::HandlePuzzleSolved()
{
}

void AREPuzzleManager::HandlePuzzleFailed()
{
}

void AREPuzzleManager::HandleSavedSolvedStateRestored()
{
	HandlePuzzleSolved();
	OnPuzzleSolved.Broadcast();
}

void AREPuzzleManager::SetPuzzleState(EREPuzzleState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	OnRep_State();
}

void AREPuzzleManager::MarkSolved()
{
	if (HasAuthority() == false || State == EREPuzzleState::Solved)
	{
		return;
	}

	SetPuzzleState(EREPuzzleState::Solved);
	HandlePuzzleSolved();
	SaveSolvedProgress();
	OnPuzzleSolved.Broadcast();
}

void AREPuzzleManager::MarkFailed()
{
	if (HasAuthority() == false || State == EREPuzzleState::Solved)
	{
		return;
	}

	SetPuzzleState(EREPuzzleState::Failed);
	HandlePuzzleFailed();
	OnPuzzleFailed.Broadcast();
}

FName AREPuzzleManager::ResolveProgressId() const
{
	return ProgressId.IsNone() == false ? ProgressId : GetFName();
}

bool AREPuzzleManager::TryRestoreSavedSolvedState()
{
	if (HasAuthority() == false || bAutoRestoreSavedSolvedState == false || bSaveSolvedState == false || IsSolved() == true)
	{
		return false;
	}

	const FName ResolvedProgressId = ResolveProgressId();
	if (ResolvedProgressId.IsNone() == true)
	{
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return false;
	}

	URELocalSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<URELocalSaveSubsystem>();
	if (IsValid(SaveSubsystem) == false || SaveSubsystem->IsPuzzleSolved(ResolvedProgressId) == false)
	{
		return false;
	}

	SetPuzzleState(EREPuzzleState::Solved);
	HandleSavedSolvedStateRestored();
	return true;
}

void AREPuzzleManager::SaveSolvedProgress() const
{
	if (HasAuthority() == false || ShouldSaveSolvedState() == false)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance) == false)
	{
		return;
	}

	if (URELocalSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<URELocalSaveSubsystem>())
	{
		SaveSubsystem->MarkPuzzleSolved(ResolveProgressId(), true);
	}
}
