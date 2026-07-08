#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathFunctionLibrary.generated.h"

UCLASS()
class ROOMESCAPE_API URETilePathFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	static FText GetDirectionText(ERETilePathDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	static FText GetAnswerChoiceText(ERETilePathAnswerChoice AnswerChoice);
};
