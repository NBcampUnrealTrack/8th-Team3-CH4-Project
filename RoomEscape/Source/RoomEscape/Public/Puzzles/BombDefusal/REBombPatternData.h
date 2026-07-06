#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "REBombPatternData.generated.h"

UCLASS(BlueprintType)
class ROOMESCAPE_API UREBombPatternData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern")
	TArray<FREBombWireDefinition> Wires;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern")
	TArray<FREBombStep> SolutionSteps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern", meta = (ClampMin = "1.0"))
	float TimeLimitSeconds = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Pattern", meta = (MultiLine = "true"))
	TArray<FText> ManualRules;

public:
	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool GetWireDefinition(int32 WireIndex, FREBombWireDefinition& OutWireDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool GetSolutionStep(int32 StepIndex, FREBombStep& OutStep) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	int32 GetStepCount() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	bool IsPatternValid() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Pattern")
	FText BuildManualText() const;
};
