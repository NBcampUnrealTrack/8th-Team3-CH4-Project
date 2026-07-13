#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "REBombPatternData.generated.h"

/**
 * All logical bomb-defusal content lives here. The integrated Blueprint owns
 * only layout (component transforms); this DataAsset owns labels, visuals,
 * initial states, time limit and the ordered solution.
 */
UCLASS(BlueprintType)
class ROOMESCAPE_API UREBombPatternData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Entries available to the WBP and matching UREBombWireComponents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern|Elements")
	TArray<FREBombWireDefinition> Wires;

	/** Entries available to the WBP and matching UREBombButtonComponents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern|Elements")
	TArray<FREBombButtonDefinition> Buttons;

	/** Ordered answer sequence validated by the server. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern|Solution")
	TArray<FREBombStep> SolutionSteps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern|Rule", meta = (ClampMin = "1.0"))
	float TimeLimitSeconds = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern|Manual", meta = (MultiLine = "true"))
	TArray<FText> ManualRules;

public:
	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool GetWireDefinition(int32 WireIndex, FREBombWireDefinition& OutWireDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool GetButtonDefinition(FName ButtonId, FREBombButtonDefinition& OutButtonDefinition) const;

	/**
	 * Backward compatibility: when Buttons is empty, unique ButtonIds referenced
	 * by SolutionSteps are exposed as default button definitions.
	 */
	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	void GetResolvedButtonDefinitions(TArray<FREBombButtonDefinition>& OutButtonDefinitions) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool GetSolutionStep(int32 StepIndex, FREBombStep& OutStep) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	int32 GetStepCount() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	int32 GetWireCount() const;

	/** Returns the resolved count, including legacy buttons generated from steps. */
	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	int32 GetButtonCount() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool IsPatternValid() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	FText BuildManualText() const;
};
