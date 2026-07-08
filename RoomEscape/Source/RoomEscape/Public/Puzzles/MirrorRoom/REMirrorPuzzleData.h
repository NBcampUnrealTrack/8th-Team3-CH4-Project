#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorPuzzleData.generated.h"

UCLASS(BlueprintType)
class ROOMESCAPE_API UREMirrorPuzzleData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Puzzle")
	EREMirrorInputType InputType = EREMirrorInputType::SymbolSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Puzzle")
	TArray<FREMirrorCueDefinition> Cues;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Puzzle|Cue Display")
	bool bShowCueOrderLabels = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Puzzle")
	TArray<FREMirrorInputOption> InputOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Puzzle")
	TArray<FREMirrorSolutionStep> SolutionSteps;

public:
	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool IsPuzzleDataValid() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	int32 GetStepCount() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool GetCueById(const FName& CueId, FREMirrorCueDefinition& OutCue) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool GetCueByIndex(int32 CueIndex, FREMirrorCueDefinition& OutCue) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool GetSolutionStep(int32 StepIndex, FREMirrorSolutionStep& OutStep) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool GetInputOptionById(const FName& InputId, FREMirrorInputOption& OutOption) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	bool GetClockInputOption(int32 ClockHour, FREMirrorInputOption& OutOption) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")	
	void GetResolvedInputOptions(TArray<FREMirrorInputOption>& OutOptions) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	FText BuildCueDisplayText(const FREMirrorCueDefinition& Cue) const;

	UFUNCTION(BlueprintPure, Category = "Mirror Puzzle")
	FText BuildSubmittedInputText(const TArray<FName>& SubmittedInputIds, const TArray<int32>& SubmittedClockHours) const;
};
