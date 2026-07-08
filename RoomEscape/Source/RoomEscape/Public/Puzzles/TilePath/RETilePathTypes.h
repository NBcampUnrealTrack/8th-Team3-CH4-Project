#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RETilePathTypes.generated.h"

UENUM(BlueprintType)
enum class ERETilePathDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right
};

UENUM(BlueprintType)
enum class ERETilePathAnswerChoice : uint8
{
	A,
	B,
	C,
	D
};

UENUM(BlueprintType)
enum class ERETilePathSessionPhase : uint8
{
	Waiting,
	Countdown,
	Playing
};

UENUM(BlueprintType)
enum class ERETilePathParticipantRole : uint8
{
	None,
	Walker,
	Guide
};

UENUM(BlueprintType)
enum class ERETilePathTileState : uint8
{
	Default,
	Locked,
	Available,
	Current,
	Passed,
	Failed,
	Solved
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FRETilePathQuestionRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERETilePathAnswerChoice CorrectAnswer = ERETilePathAnswerChoice::A;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FRETilePathRevealedMove
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StepIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint FromCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint ToCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERETilePathDirection Direction = ERETilePathDirection::None;
};
