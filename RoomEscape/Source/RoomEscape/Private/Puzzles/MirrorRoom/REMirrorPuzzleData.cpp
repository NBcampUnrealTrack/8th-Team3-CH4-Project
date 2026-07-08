#include "Puzzles/MirrorRoom/REMirrorPuzzleData.h"
#include "Puzzles/MirrorRoom/REMirrorRoomFunctionLibrary.h"

bool UREMirrorPuzzleData::IsPuzzleDataValid() const
{
	if (SolutionSteps.Num() <= 0)
	{
		return false;
	}

	if (InputType == EREMirrorInputType::SymbolSequence)
	{
		for (const FREMirrorSolutionStep& Step : SolutionSteps)
		{
			if (Step.ExpectedInputId.IsNone() == true)
			{
				return false;
			}

			if (InputOptions.Num() > 0)
			{
				FREMirrorInputOption Option;
				if (GetInputOptionById(Step.ExpectedInputId, Option) == false)
				{
					return false;
				}
			}
		}
	}
	else
	{
		for (const FREMirrorSolutionStep& Step : SolutionSteps)
		{
			if (Step.ExpectedClockHour < 1 || Step.ExpectedClockHour > 12)
			{
				return false;
			}
		}
	}

	return true;
}

int32 UREMirrorPuzzleData::GetStepCount() const
{
	return SolutionSteps.Num();
}

bool UREMirrorPuzzleData::GetCueById(const FName& CueId, FREMirrorCueDefinition& OutCue) const
{
	for (const FREMirrorCueDefinition& Cue : Cues)
	{
		if (Cue.CueId == CueId)
		{
			OutCue = Cue;
			return true;
		}
	}

	return false;
}

bool UREMirrorPuzzleData::GetCueByIndex(int32 CueIndex, FREMirrorCueDefinition& OutCue) const
{
	if (Cues.IsValidIndex(CueIndex) == false)
	{
		return false;
	}

	OutCue = Cues[CueIndex];
	return true;
}

bool UREMirrorPuzzleData::GetSolutionStep(int32 StepIndex, FREMirrorSolutionStep& OutStep) const
{
	if (SolutionSteps.IsValidIndex(StepIndex) == false)
	{
		return false;
	}

	OutStep = SolutionSteps[StepIndex];
	return true;
}

bool UREMirrorPuzzleData::GetInputOptionById(const FName& InputId, FREMirrorInputOption& OutOption) const
{
	for (const FREMirrorInputOption& Option : InputOptions)
	{
		if (Option.InputId == InputId)
		{
			OutOption = Option;
			return true;
		}
	}

	return false;
}

bool UREMirrorPuzzleData::GetClockInputOption(int32 ClockHour, FREMirrorInputOption& OutOption) const
{
	const int32 NormalizedHour = UREMirrorRoomFunctionLibrary::NormalizeClockHour(ClockHour);
	for (const FREMirrorInputOption& Option : InputOptions)
	{
		if (UREMirrorRoomFunctionLibrary::NormalizeClockHour(Option.ClockHour) == NormalizedHour)
		{
			OutOption = Option;
			return true;
		}
	}

	OutOption.InputId = FName(*FString::Printf(TEXT("Hour_%02d"), NormalizedHour));
	OutOption.ClockHour = NormalizedHour;
	OutOption.DisplayText = FText::Format(FText::FromString(TEXT("{0}시")), FText::AsNumber(NormalizedHour));
	return true;
}

void UREMirrorPuzzleData::GetResolvedInputOptions(TArray<FREMirrorInputOption>& OutOptions) const
{
	OutOptions.Reset();

	if (InputOptions.Num() > 0)
	{
		OutOptions = InputOptions;
		return;
	}

	if (InputType == EREMirrorInputType::ClockHourSequence)
	{
		for (int32 Hour = 1; Hour <= 12; ++Hour)
		{
			FREMirrorInputOption Option;
			Option.InputId = FName(*FString::Printf(TEXT("Hour_%02d"), Hour));
			Option.ClockHour = Hour;
			Option.DisplayText = FText::Format(FText::FromString(TEXT("{0}시")), FText::AsNumber(Hour));
			OutOptions.Add(Option);
		}
		return;
	}

	TSet<FName> AddedIds;
	for (const FREMirrorSolutionStep& Step : SolutionSteps)
	{
		if (Step.ExpectedInputId.IsNone() == true || AddedIds.Contains(Step.ExpectedInputId) == true)
		{
			continue;
		}

		FREMirrorInputOption Option;
		Option.InputId = Step.ExpectedInputId;
		Option.DisplayText = FText::FromString(Step.ExpectedInputId.ToString());
		OutOptions.Add(Option);
		AddedIds.Add(Step.ExpectedInputId);
	}
}

FText UREMirrorPuzzleData::BuildCueDisplayText(const FREMirrorCueDefinition& Cue) const
{
	FText MirroredValue;
	if (Cue.CueType == EREMirrorCueType::ClockHour)
	{
		const int32 MirroredHour = UREMirrorRoomFunctionLibrary::MirrorClockHour(Cue.ActualClockHour, Cue.MirrorAxis);
		MirroredValue = FText::Format(FText::FromString(TEXT("{0}시")), FText::AsNumber(MirroredHour));
	}
	else
	{
		MirroredValue = FText::FromString(UREMirrorRoomFunctionLibrary::MirrorText(Cue.ActualText.ToString(), Cue.MirrorAxis));
	}

	FString Result;
	if (bShowCueOrderLabels == true && Cue.bShowOrderLabel == true && Cue.DisplayOrder > 0)
	{
		Result += FString::Printf(TEXT("%d. "), Cue.DisplayOrder);
	}

	const FString Prefix = Cue.DisplayPrefix.ToString();
	if (Prefix.TrimStartAndEnd().IsEmpty() == false)
	{
		Result += Prefix;
		Result += TEXT(" ");
	}

	Result += MirroredValue.ToString();
	return FText::FromString(Result);
}

FText UREMirrorPuzzleData::BuildSubmittedInputText(const TArray<FName>& SubmittedInputIds, const TArray<int32>& SubmittedClockHours) const
{
	TArray<FString> Parts;
	if (InputType == EREMirrorInputType::ClockHourSequence)
	{
		for (const int32 Hour : SubmittedClockHours)
		{
			Parts.Add(FString::Printf(TEXT("%d시"), UREMirrorRoomFunctionLibrary::NormalizeClockHour(Hour)));
		}
	}
	else
	{
		for (const FName& InputId : SubmittedInputIds)
		{
			FREMirrorInputOption Option;
			if (GetInputOptionById(InputId, Option) == true && Option.DisplayText.IsEmpty() == false)
			{
				Parts.Add(Option.DisplayText.ToString());
			}
			else
			{
				Parts.Add(InputId.ToString());
			}
		}
	}

	return FText::FromString(FString::Join(Parts, TEXT("  ")));
}
