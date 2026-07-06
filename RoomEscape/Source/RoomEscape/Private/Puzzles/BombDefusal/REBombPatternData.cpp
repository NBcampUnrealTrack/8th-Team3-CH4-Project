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
		else if (Step.StepType == EREBombStepType::HoldButton)
		{
			if (Step.ButtonId.IsNone() == true || Step.RequiredHoldSeconds <= 0.0f)
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
	for (int32 Index = 0; Index < ManualRules.Num(); ++Index)
	{
		const FString RuleString = ManualRules[Index].ToString();
		if (RuleString.IsEmpty() == true)
		{
			continue;
		}

		if (CombinedRules.IsEmpty() == false)
		{
			CombinedRules += TEXT("\n");
		}
		CombinedRules += FString::Printf(TEXT("%d. %s"), Index + 1, *RuleString);
	}

	return FText::FromString(CombinedRules);
}
