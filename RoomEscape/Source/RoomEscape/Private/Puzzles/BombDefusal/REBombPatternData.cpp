#include "Puzzles/BombDefusal/REBombPatternData.h"

bool UREBombPatternData::GetWireDefinition(int32 WireIndex, FREBombWireDefinition& OutWireDefinition) const
{
	for (const FREBombWireDefinition& WireDefinition : Wires)
	{
		if (WireDefinition.WireIndex == WireIndex)
		{
			OutWireDefinition = WireDefinition;
			return true;
		}
	}

	return false;
}

bool UREBombPatternData::GetSolutionStep(int32 StepIndex, FREBombStep& OutStep) const
{
	if (SolutionSteps.IsValidIndex(StepIndex) == false)
	{
		return false;
	}

	OutStep = SolutionSteps[StepIndex];
	return true;
}

int32 UREBombPatternData::GetStepCount() const
{
	return SolutionSteps.Num();
}

bool UREBombPatternData::IsPatternValid() const
{
	if (TimeLimitSeconds <= 0.0f || SolutionSteps.Num() <= 0)
	{
		return false;
	}

	for (const FREBombStep& Step : SolutionSteps)
	{
		if (Step.StepType == EREBombStepType::CutWire)
		{
			FREBombWireDefinition WireDefinition;
			if (GetWireDefinition(Step.WireIndex, WireDefinition) == false)
			{
				return false;
			}
		}
		else if (Step.StepType == EREBombStepType::ButtonState)
		{
			if (Step.ButtonId.IsNone() == true)
			{
				return false;
			}
		}
	}

	return true;
}

FText UREBombPatternData::BuildManualText() const
{
	FString CombinedRules;
	for (const FText& ManualRule : ManualRules)
	{
		const FString RuleString = ManualRule.ToString();
		if (RuleString.TrimStartAndEnd().IsEmpty() == true)
		{
			continue;
		}

		if (CombinedRules.IsEmpty() == false)
		{
			CombinedRules += TEXT("\n");
		}
		CombinedRules += RuleString;
	}

	return FText::FromString(CombinedRules);
}
