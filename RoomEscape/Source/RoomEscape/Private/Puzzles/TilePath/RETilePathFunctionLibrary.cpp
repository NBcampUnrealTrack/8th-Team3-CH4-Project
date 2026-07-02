#include "Puzzles/TilePath/RETilePathFunctionLibrary.h"

FText URETilePathFunctionLibrary::GetDirectionText(ERETilePathDirection Direction)
{
	switch (Direction)
	{
	case ERETilePathDirection::Up:
		return FText::FromString(TEXT("↑"));
	case ERETilePathDirection::Down:
		return FText::FromString(TEXT("↓"));
	case ERETilePathDirection::Left:
		return FText::FromString(TEXT("←"));
	case ERETilePathDirection::Right:
		return FText::FromString(TEXT("→"));
	default:
		return FText::FromString(TEXT("없음"));
	}
}

FText URETilePathFunctionLibrary::GetAnswerChoiceText(ERETilePathAnswerChoice AnswerChoice)
{
	switch (AnswerChoice)
	{
	case ERETilePathAnswerChoice::A:
		return FText::FromString(TEXT("A"));
	case ERETilePathAnswerChoice::B:
		return FText::FromString(TEXT("B"));
	case ERETilePathAnswerChoice::C:
		return FText::FromString(TEXT("C"));
	case ERETilePathAnswerChoice::D:
		return FText::FromString(TEXT("D"));
	default:
		return FText::GetEmpty();
	}
}
