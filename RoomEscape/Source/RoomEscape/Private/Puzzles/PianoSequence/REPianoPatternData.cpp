// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/PianoSequence/REPianoPatternData.h"

#include "Puzzles/PianoSequence/REPianoSequenceTypes.h"

const FREPianoNote* UREPianoPatternData::FindNote(int32 NoteIndex) const
{
	return NoteScale.FindByPredicate([NoteIndex](const FREPianoNote& Note)
	{
		return Note.NoteIndex == NoteIndex;
	});
}

bool UREPianoPatternData::IsPatternValid() const
{
	return NoteScale.Num() > 0 && MelodyLength > 0;
}
