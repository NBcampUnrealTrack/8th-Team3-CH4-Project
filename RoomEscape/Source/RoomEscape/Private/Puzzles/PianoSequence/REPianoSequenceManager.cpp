// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/PianoSequence/REPianoSequenceManager.h"
#include "Puzzles/PianoSequence/REPianoPatternData.h"
#include "Puzzles/PianoSequence/REPianoInstrument.h"
#include "Puzzles/PianoSequence/REPianoBoard.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Game/RENotifySubsystem.h"
#include "AbilitySystem/NativeGameplayTags.h"

AREPianoSequenceManager::AREPianoSequenceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AREPianoSequenceManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Instrument);
	DOREPLIFETIME(ThisClass, Board);
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
	// bNeedsNewMelody - 오답 후에는 A가 새 멜로디를 재생하기 전까지 B의 입력을 받지 않는다
	if (HasAuthority() == false || IsActive() == false || IsValid(PatternData) == false || bNeedsNewMelody == true)
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

#if !UE_BUILD_SHIPPING
	// 디버그 전용 - 정답 멜로디와, 실제로 눌러야 할 슬롯(버튼) 순서를 로그에 출력
	FString MelodyStr;
	FString SlotStr;
	for (int32 i = 0; i < CurrentMelody.Num(); ++i)
	{
		const int32 NoteIndex = CurrentMelody[i];

		FString Label = FString::FromInt(NoteIndex);
		if (const FREPianoNote* Note = PatternData->FindNote(NoteIndex))
		{
			if (Note->NoteLabel.IsEmpty() == false)
			{
				Label = Note->NoteLabel.ToString();
			}
		}
		MelodyStr += (i > 0 ? TEXT(" - ") : TEXT("")) + Label;

		// 셔플이 반영된 화면 슬롯 번호 (0..N-1). Board가 아직 없으면 ?
		FString SlotLabel = TEXT("?");
		if (IsValid(Board) == true)
		{
			const int32 SlotIndex = Board->GetSlotIndexForNote(NoteIndex);
			SlotLabel = (SlotIndex == INDEX_NONE) ? TEXT("?") : FString::FromInt(SlotIndex);
		}
		SlotStr += (i > 0 ? TEXT(" - ") : TEXT("")) + SlotLabel;
	}
	if (URENotifySubsystem* Notify = URENotifySubsystem::GetInstance(this))
	{
		Notify->NotifyEvent(RETag::Event::Debug::Piano, FString::Printf(TEXT("정답 멜로디 (%d): %s"), CurrentMelody.Num(), *MelodyStr));
		Notify->NotifyEvent(RETag::Event::Debug::Piano, FString::Printf(TEXT("눌러야 할 버튼 순서: %s"), *SlotStr));
	}
#endif
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

#if !UE_BUILD_SHIPPING
	// 디버그 전용 - 판정 결과와 입력한 음을 로그에 출력 (Reset 전에 찍어야 함)
	if (URENotifySubsystem* Notify = URENotifySubsystem::GetInstance(this))
	{
		auto BuildNoteString = [this](const TArray<int32>& Notes) -> FString
		{
			FString Result;
			for (int32 i = 0; i < Notes.Num(); ++i)
			{
				FString Label = FString::FromInt(Notes[i]);
				if (const FREPianoNote* Note = PatternData->FindNote(Notes[i]))
				{
					if (Note->NoteLabel.IsEmpty() == false)
					{
						Label = Note->NoteLabel.ToString();
					}
				}
				Result += (i > 0 ? TEXT(" - ") : TEXT("")) + Label;
			}
			return Result;
		};

		if (bCorrect == true)
		{
			Notify->NotifyEvent(RETag::Event::Debug::Piano, FString::Printf(TEXT("판정: 정답! (%s)"), *BuildNoteString(PressedBuffer)));
		}
		else
		{
			Notify->NotifyEvent(RETag::Event::Debug::Piano, FString::Printf(TEXT("판정: 오답 - 입력: %s / 정답: %s"), *BuildNoteString(PressedBuffer), *BuildNoteString(CurrentMelody)));
		}
	}
#endif

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



