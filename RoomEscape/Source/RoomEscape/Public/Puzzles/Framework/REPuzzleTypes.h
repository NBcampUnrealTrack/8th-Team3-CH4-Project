#pragma once

#include "CoreMinimal.h"
#include "REPuzzleTypes.generated.h"

UENUM(BlueprintType)
enum class EREPuzzleState : uint8
{
	Locked,
	Active,
	Solved,
	Failed
};
