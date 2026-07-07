// Fill out your copyright notice in the Description page of Project Settings.

#include "Puzzles/PianoSequence/REPianoInstrument.h"
#include "Puzzles/PianoSequence/REPianoSequenceManager.h"

AREPianoInstrument::AREPianoInstrument()
{
}

void AREPianoInstrument::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		if (AREPianoSequenceManager* Manager = GetPianoManager())
		{
			Manager->RegisterInstrument(this);
		}
	}
}

void AREPianoInstrument::HandleInteract(AActor* Interactor)
{
	if (AREPianoSequenceManager* Manager = GetPianoManager())
	{
		Manager->RequestPlayMelody(Interactor);
	}
}

void AREPianoInstrument::PlayNote(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration)
{
	ReceiveNotePlayed(NoteIndex, GlowColor, NoteSound, Duration);
}

AREPianoSequenceManager* AREPianoInstrument::GetPianoManager() const
{
	return Cast<AREPianoSequenceManager>(GetPuzzleManager());
}
