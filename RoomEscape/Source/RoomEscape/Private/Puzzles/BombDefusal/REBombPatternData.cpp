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

bool UREBombPatternData::GetButtonDefinition(FName ButtonId, FREBombButtonDefinition& OutButtonDefinition) const
{
	for (const FREBombButtonDefinition& ButtonDefinition : Buttons)
	{
		if (ButtonDefinition.ButtonId == ButtonId)
		{
			OutButtonDefinition = ButtonDefinition;
			return true;
		}
	}

	// Legacy DataAssets had no Buttons array. Preserve them by resolving any
	// ButtonId referenced by the solution into a default definition.
	if (Buttons.IsEmpty() == true && ButtonId.IsNone() == false)
	{
		for (const FREBombStep& Step : SolutionSteps)
		{
			if (Step.StepType == EREBombStepType::ButtonState && Step.ButtonId == ButtonId)
			{
				OutButtonDefinition = FREBombButtonDefinition();
				OutButtonDefinition.ButtonId = ButtonId;
				OutButtonDefinition.ButtonLabel = FText::FromString(ButtonId.ToString());
				return true;
			}
		}
	}

	return false;
}

void UREBombPatternData::GetResolvedButtonDefinitions(TArray<FREBombButtonDefinition>& OutButtonDefinitions) const
{
	OutButtonDefinitions = Buttons;
	if (Buttons.IsEmpty() == false)
	{
		return;
	}

	for (const FREBombStep& Step : SolutionSteps)
	{
		if (Step.StepType != EREBombStepType::ButtonState || Step.ButtonId.IsNone() == true)
		{
			continue;
		}

		const bool bAlreadyAdded = OutButtonDefinitions.ContainsByPredicate([&Step](const FREBombButtonDefinition& Definition)
		{
			return Definition.ButtonId == Step.ButtonId;
		});

		if (bAlreadyAdded == true)
		{
			continue;
		}

		FREBombButtonDefinition GeneratedDefinition;
		GeneratedDefinition.ButtonId = Step.ButtonId;
		GeneratedDefinition.ButtonLabel = FText::FromString(Step.ButtonId.ToString());
		OutButtonDefinitions.Add(GeneratedDefinition);
	}
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

int32 UREBombPatternData::GetWireCount() const
{
	return Wires.Num();
}

int32 UREBombPatternData::GetButtonCount() const
{
	TArray<FREBombButtonDefinition> ResolvedButtons;
	GetResolvedButtonDefinitions(ResolvedButtons);
	return ResolvedButtons.Num();
}

bool UREBombPatternData::IsPatternValid() const
{
	if (TimeLimitSeconds <= 0.0f || SolutionSteps.IsEmpty() == true)
	{
		return false;
	}

	TSet<int32> UniqueWireIndices;
	for (const FREBombWireDefinition& WireDefinition : Wires)
	{
		if (WireDefinition.WireIndex < 0 || UniqueWireIndices.Contains(WireDefinition.WireIndex) == true)
		{
			return false;
		}

		UniqueWireIndices.Add(WireDefinition.WireIndex);
	}

	TArray<FREBombButtonDefinition> ResolvedButtons;
	GetResolvedButtonDefinitions(ResolvedButtons);

	TSet<FName> UniqueButtonIds;
	TMap<FName, bool> SimulatedButtonStates;
	for (const FREBombButtonDefinition& ButtonDefinition : ResolvedButtons)
	{
		if (ButtonDefinition.ButtonId.IsNone() == true || UniqueButtonIds.Contains(ButtonDefinition.ButtonId) == true)
		{
			return false;
		}

		UniqueButtonIds.Add(ButtonDefinition.ButtonId);
		SimulatedButtonStates.Add(ButtonDefinition.ButtonId, ButtonDefinition.bInitialPressed);
	}

	// Simulate the full sequence so an impossible DA is rejected before play.
	TSet<int32> SimulatedCutWires;
	for (const FREBombStep& Step : SolutionSteps)
	{
		switch (Step.StepType)
		{
		case EREBombStepType::CutWire:
		{
			FREBombWireDefinition WireDefinition;
			if (GetWireDefinition(Step.WireIndex, WireDefinition) == false
				|| SimulatedCutWires.Contains(Step.WireIndex) == true)
			{
				return false;
			}

			SimulatedCutWires.Add(Step.WireIndex);
			break;
		}

		case EREBombStepType::ButtonState:
		{
			FREBombButtonDefinition ButtonDefinition;
			bool* CurrentPressedState = SimulatedButtonStates.Find(Step.ButtonId);
			if (Step.ButtonId.IsNone() == true
				|| GetButtonDefinition(Step.ButtonId, ButtonDefinition) == false
				|| CurrentPressedState == nullptr)
			{
				return false;
			}

			const bool bNextPressedState = !(*CurrentPressedState);
			if (bNextPressedState != Step.bRequiredButtonPressed)
			{
				return false;
			}

			*CurrentPressedState = bNextPressedState;
			break;
		}

		default:
			return false;
		}
	}

	return true;
}

FText UREBombPatternData::BuildManualText() const
{
	FString CombinedRules;
	for (const FText& ManualRule : ManualRules)
	{
		const FString RuleString = ManualRule.ToString().TrimStartAndEnd();
		if (RuleString.IsEmpty() == true)
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
