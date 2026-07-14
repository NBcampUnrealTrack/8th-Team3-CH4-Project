#include "Puzzles/LockAndGlow/RELockAndGlowClueManager.h"

#include "Containers/AllowShrinking.h"
#include "Puzzles/LockAndGlow/REDialLockDevice.h"
#include "Puzzles/LockAndGlow/REGlowPaintClueActor.h"

ARELockAndGlowClueManager::ARELockAndGlowClueManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARELockAndGlowClueManager::BeginPlay()
{
	if (CorrectCodePolicy == ERELockAndGlowCorrectCodePolicy::ValidateOnly && bDisableProgressPersistenceWhenValidateOnly == true)
	{
		/*
		 * Prototype validation should ignore old solved progress and avoid writing new
		 * progress, otherwise one successful test can permanently disable interaction
		 * in the next PIE run.
		 */
		bSaveSolvedState = false;
		bAutoRestoreSavedSolvedState = false;
	}

	Super::BeginPlay();
}

int32 ARELockAndGlowClueManager::GetCodeLength() const
{
	return FMath::Clamp(CodeLength, 1, 8);
}

FString ARELockAndGlowClueManager::GetSolutionCode() const
{
	return NormalizeSolutionCode();
}

bool ARELockAndGlowClueManager::IsValidateOnlyMode() const
{
	return CorrectCodePolicy == ERELockAndGlowCorrectCodePolicy::ValidateOnly;
}

void ARELockAndGlowClueManager::BuildSolutionDigits(TArray<int32>& OutDigits) const
{
	OutDigits.Reset();
	const FString NormalizedCode = NormalizeSolutionCode();
	for (int32 CharIndex = 0; CharIndex < NormalizedCode.Len(); ++CharIndex)
	{
		const TCHAR DigitChar = NormalizedCode[CharIndex];
		OutDigits.Add(DigitChar >= TCHAR('0') && DigitChar <= TCHAR('9') ? static_cast<int32>(DigitChar - TCHAR('0')) : 0);
	}
}

void ARELockAndGlowClueManager::RegisterLockDevice(AREDialLockDevice* InLockDevice)
{
	if (IsValid(InLockDevice) == false)
	{
		return;
	}

	LockDevices.AddUnique(InLockDevice);
	if (HasAuthority() == true)
	{
		InLockDevice->ApplyServerCodeLength(GetCodeLength());
		InLockDevice->ApplyServerUnlockedState(IsSolved());
	}
}

void ARELockAndGlowClueManager::RegisterGlowClue(AREGlowPaintClueActor* InGlowClue)
{
	if (IsValid(InGlowClue) == false)
	{
		return;
	}

	GlowClues.AddUnique(InGlowClue);
	InGlowClue->SetLockAndGlowManager(this);
}

bool ARELockAndGlowClueManager::SubmitCode(AREDialLockDevice* SourceDevice, AActor* Interactor, const TArray<int32>& SubmittedDigits)
{
	if (HasAuthority() == false || IsActive() == false || IsValid(SourceDevice) == false)
	{
		return false;
	}

	const bool bCorrect = AreDigitsMatchingSolution(SubmittedDigits);
	const FText ResultMessage = bCorrect == true ? CorrectCodeMessage : IncorrectCodeMessage;
	MulticastDialLockInputResult(SourceDevice, bCorrect, ResultMessage);

	if (bCorrect == true && CorrectCodePolicy == ERELockAndGlowCorrectCodePolicy::SolvePuzzle)
	{
		MarkSolved();
	}

	return bCorrect;
}

void ARELockAndGlowClueManager::HandlePuzzleActivated()
{
	ApplyRuntimeStateToActors();
}

void ARELockAndGlowClueManager::HandlePuzzleLocked()
{
	ApplyRuntimeStateToActors();
}

void ARELockAndGlowClueManager::HandlePuzzleSolved()
{
	ApplyRuntimeStateToActors();
}

void ARELockAndGlowClueManager::MulticastDialLockInputResult_Implementation(AActor* SourceActor, bool bCorrect, const FText& ResultMessage)
{
	OnDialLockInputResult.Broadcast(SourceActor, bCorrect, ResultMessage);
	ReceiveDialLockInputResult(SourceActor, bCorrect, ResultMessage);
}

FString ARELockAndGlowClueManager::NormalizeSolutionCode() const
{
	FString DigitsOnly;
	for (int32 CharIndex = 0; CharIndex < SolutionCode.Len(); ++CharIndex)
	{
		const TCHAR CurrentChar = SolutionCode[CharIndex];
		if (CurrentChar >= TCHAR('0') && CurrentChar <= TCHAR('9'))
		{
			DigitsOnly.AppendChar(CurrentChar);
		}
	}

	const int32 DesiredLength = GetCodeLength();
	while (DigitsOnly.Len() < DesiredLength)
	{
		DigitsOnly.AppendChar(TCHAR('0'));
	}

	if (DigitsOnly.Len() > DesiredLength)
	{
		DigitsOnly.LeftInline(DesiredLength, EAllowShrinking::No);
	}

	return DigitsOnly;
}

bool ARELockAndGlowClueManager::AreDigitsMatchingSolution(const TArray<int32>& SubmittedDigits) const
{
	TArray<int32> SolutionDigits;
	BuildSolutionDigits(SolutionDigits);
	if (SubmittedDigits.Num() != SolutionDigits.Num())
	{
		return false;
	}

	for (int32 DigitIndex = 0; DigitIndex < SolutionDigits.Num(); ++DigitIndex)
	{
		if (FMath::Clamp(SubmittedDigits[DigitIndex], 0, 9) != SolutionDigits[DigitIndex])
		{
			return false;
		}
	}

	return true;
}

void ARELockAndGlowClueManager::ApplyRuntimeStateToActors()
{
	if (HasAuthority() == true)
	{
		for (const TObjectPtr<AREDialLockDevice>& LockDevicePtr : LockDevices)
		{
			AREDialLockDevice* LockDevice = LockDevicePtr.Get();
			if (IsValid(LockDevice) == true)
			{
				LockDevice->ApplyServerCodeLength(GetCodeLength());
				LockDevice->ApplyServerUnlockedState(IsSolved());
			}
		}
	}

	for (const TObjectPtr<AREGlowPaintClueActor>& GlowCluePtr : GlowClues)
	{
		AREGlowPaintClueActor* GlowClue = GlowCluePtr.Get();
		if (IsValid(GlowClue) == true)
		{
			GlowClue->RefreshClueVisibility();
		}
	}
}
