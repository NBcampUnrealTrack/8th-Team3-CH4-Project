#include "Puzzles/MirrorRoom/REMirrorRoomFunctionLibrary.h"

namespace REMirrorRoom
{
	static TCHAR MirrorCharHorizontal(TCHAR Character)
	{
		switch (Character)
		{
		case TEXT('b'): return TEXT('d');
		case TEXT('d'): return TEXT('b');
		case TEXT('p'): return TEXT('q');
		case TEXT('q'): return TEXT('p');
		case TEXT('B'): return TEXT('D');
		case TEXT('D'): return TEXT('B');
		case TEXT('P'): return TEXT('Q');
		case TEXT('Q'): return TEXT('P');
		case TEXT('6'): return TEXT('9');
		case TEXT('9'): return TEXT('6');
		case TEXT('<'): return TEXT('>');
		case TEXT('>'): return TEXT('<');
		case TEXT('('): return TEXT(')');
		case TEXT(')'): return TEXT('(');
		case TEXT('['): return TEXT(']');
		case TEXT(']'): return TEXT('[');
		case TEXT('{'): return TEXT('}');
		case TEXT('}'): return TEXT('{');
		case TEXT('/'): return TEXT('\\');
		case TEXT('\\'): return TEXT('/');
		default: return Character;
		}
	}

	static TCHAR MirrorCharVertical(TCHAR Character)
	{
		switch (Character)
		{
		case TEXT('b'): return TEXT('p');
		case TEXT('p'): return TEXT('b');
		case TEXT('d'): return TEXT('q');
		case TEXT('q'): return TEXT('d');
		case TEXT('B'): return TEXT('P');
		case TEXT('P'): return TEXT('B');
		case TEXT('D'): return TEXT('Q');
		case TEXT('Q'): return TEXT('D');
		case TEXT('6'): return TEXT('9');
		case TEXT('9'): return TEXT('6');
		case TEXT('^'): return TEXT('v');
		case TEXT('v'): return TEXT('^');
		case TEXT('V'): return TEXT('^');
		default: return Character;
		}
	}
}

FString UREMirrorRoomFunctionLibrary::MirrorText(const FString& Source, EREMirrorAxis MirrorAxis)
{
	if (MirrorAxis == EREMirrorAxis::None || Source.IsEmpty() == true)
	{
		return Source;
	}

	FString Result;
	if (MirrorAxis == EREMirrorAxis::Horizontal || MirrorAxis == EREMirrorAxis::Rotate180)
	{
		for (int32 Index = Source.Len() - 1; Index >= 0; --Index)
		{
			TCHAR MirroredChar = REMirrorRoom::MirrorCharHorizontal(Source[Index]);
			if (MirrorAxis == EREMirrorAxis::Rotate180)
			{
				MirroredChar = REMirrorRoom::MirrorCharVertical(MirroredChar);
			}
			Result.AppendChar(MirroredChar);
		}
		return Result;
	}

	for (int32 Index = 0; Index < Source.Len(); ++Index)
	{
		Result.AppendChar(REMirrorRoom::MirrorCharVertical(Source[Index]));
	}

	return Result;
}

int32 UREMirrorRoomFunctionLibrary::MirrorClockHour(int32 ActualHour, EREMirrorAxis MirrorAxis)
{
	const int32 NormalizedHour = NormalizeClockHour(ActualHour);
	int32 MirroredHour = NormalizedHour;

	if (MirrorAxis == EREMirrorAxis::Horizontal)
	{
		MirroredHour = 12 - NormalizedHour;
	}
	else if (MirrorAxis == EREMirrorAxis::Vertical)
	{
		MirroredHour = 6 - NormalizedHour;
	}
	else if (MirrorAxis == EREMirrorAxis::Rotate180)
	{
		MirroredHour = NormalizedHour + 6;
	}

	return NormalizeClockHour(MirroredHour);
}

int32 UREMirrorRoomFunctionLibrary::NormalizeClockHour(int32 Hour)
{
	int32 NormalizedHour = Hour % 12;
	if (NormalizedHour <= 0)
	{
		NormalizedHour += 12;
	}
	return NormalizedHour;
}

FText UREMirrorRoomFunctionLibrary::GetMirrorAxisText(EREMirrorAxis MirrorAxis)
{
	switch (MirrorAxis)
	{
	case EREMirrorAxis::Horizontal:
		return FText::FromString(TEXT("좌우 반전"));
	case EREMirrorAxis::Vertical:
		return FText::FromString(TEXT("상하 반전"));
	case EREMirrorAxis::Rotate180:
		return FText::FromString(TEXT("180도 회전"));
	default:
		return FText::FromString(TEXT("반전 없음"));
	}
}
