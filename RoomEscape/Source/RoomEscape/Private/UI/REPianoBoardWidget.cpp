// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/REPianoBoardWidget.h"
#include "Puzzles/PianoSequence/REPianoBoard.h"

void UREPianoBoardWidget::InitializeBoard(AREPianoBoard* InBoard)
{
	BoardRef = InBoard;
}
