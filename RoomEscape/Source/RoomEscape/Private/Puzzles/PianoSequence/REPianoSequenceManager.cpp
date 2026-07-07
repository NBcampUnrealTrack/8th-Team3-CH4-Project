// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/PianoSequence/REPianoSequenceManager.h"
#include "Puzzles/PianoSequence/REPianoPatternData.h"
#include "Puzzles/PianoSequence/REPianoInstrument.h"
#include "Puzzles/PianoSequence/REPianoBoard.h"
#include "TimerManager.h"

AREPianoSequenceManager::AREPianoSequenceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AREPianoSequenceManager::RegisterInstrument(AREPianoInstrument* InInstrument)
{
	Instrument = InInstrument;
}

void AREPianoSequenceManager::RegisterBoard(AREPianoBoard* InBoard)
{
	Board = InBoard;
}

void AREPianoSequenceManager::RequestPlayMelody(AActor* Interactor)
{
	if (HasAuthority() == false || IsActive() == false || IsValid(PatternData) == false || bIsPlayingBack == true)
	{
		return;
	}

	if (bNeedsNewMelody == true || CurrentMelody.Num() == 0)
	{
		GenerateNewMelody();
	}

	bIsPlayingBack = true;
	PlaybackNoteCursor = 0;
	PlayNextNoteInPlayback();
}

void AREPianoSequenceManager::SubmitKeyNote(AREPianoBoard* InBoard, int32 NoteIndex, AActor* Interactor)
{
	if (HasAuthority() == false || IsActive() == false || IsValid(PatternData) == false)
	{
		return;
	}

	MulticastKeyFeedback(NoteIndex);

	PressedBuffer.Add(NoteIndex);

	if (PressedBuffer.Num() < PatternData->MelodyLength)
	{
		return;
	}

	EvaluateAttempt(Interactor);
}

bool AREPianoSequenceManager::CanActivatePuzzle() const
{
	return Super::CanActivatePuzzle() == true && IsValid(PatternData) == true && PatternData->IsPatternValid() == true;
}

void AREPianoSequenceManager::HandlePuzzleActivated()
{
	Super::HandlePuzzleActivated();

	CurrentMelody.Reset();
	PressedBuffer.Reset();
	bNeedsNewMelody = true;
	bIsPlayingBack = false;
}

void AREPianoSequenceManager::HandlePuzzleLocked()
{
	Super::HandlePuzzleLocked();

	GetWorldTimerManager().ClearTimer(PlaybackTimerHandle);
	bIsPlayingBack = false;
}

void AREPianoSequenceManager::HandlePuzzleSolved()
{
	Super::HandlePuzzleSolved();

	GetWorldTimerManager().ClearTimer(PlaybackTimerHandle);
	bIsPlayingBack = false;
}

void AREPianoSequenceManager::GenerateNewMelody()
{
	CurrentMelody.Reset();

	for (int32 Index = 0; Index < PatternData->MelodyLength; ++Index)
	{
		const int32 RandomArrayIndex = FMath::RandRange(0, PatternData->NoteScale.Num() - 1);
		CurrentMelody.Add(PatternData->NoteScale[RandomArrayIndex].NoteIndex);
	}

	bNeedsNewMelody = false;
}

void AREPianoSequenceManager::PlayNextNoteInPlayback()
{
	if (PlaybackNoteCursor >= CurrentMelody.Num())
	{
		bIsPlayingBack = false;
		return;
	}

	const int32 NoteIndex = CurrentMelody[PlaybackNoteCursor];
	MulticastPlayNote(NoteIndex, PatternData->ShowDuration);
	PlaybackNoteCursor++;

	GetWorldTimerManager().SetTimer(PlaybackTimerHandle, this, &ThisClass::PlayNextNoteInPlayback, PatternData->ShowDuration, false);
}

void AREPianoSequenceManager::EvaluateAttempt(AActor* Interactor)
{
	const bool bCorrect = (PressedBuffer == CurrentMelody);

	MulticastAttemptResult(bCorrect);

	PressedBuffer.Reset();

	if (bCorrect == true)
	{
		MarkSolved();
	}
	else
	{
		bNeedsNewMelody = true;
	}
}

void AREPianoSequenceManager::MulticastPlayNote_Implementation(int32 NoteIndex, float Duration)
{
	if (IsValid(Instrument) == false || IsValid(PatternData) == false)
	{
		return;
	}

	if (const FREPianoNote* Note = PatternData->FindNote(NoteIndex))
	{
		Instrument->PlayNote(Note->NoteIndex, Note->GlowColor, Note->NoteSound, Duration);
	}
}

void AREPianoSequenceManager::MulticastKeyFeedback_Implementation(int32 NoteIndex)
{
	if (IsValid(Board) == false || IsValid(PatternData) == false)
	{
		return;
	}

	if (const FREPianoNote* Note = PatternData->FindNote(NoteIndex))
	{
		Board->PlayNote(Note->NoteIndex, Note->GlowColor, Note->NoteSound, PatternData->ShowDuration);
	}
}

void AREPianoSequenceManager::MulticastAttemptResult_Implementation(bool bCorrect)
{
	OnPianoAttemptResult.Broadcast(bCorrect);
}



