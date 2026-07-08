#pragma once

#include "CoreMinimal.h"
#include "REMirrorRoomTypes.generated.h"

UENUM(BlueprintType)
enum class EREMirrorCueType : uint8
{
	Text,
	ClockHour
};

UENUM(BlueprintType)
enum class EREMirrorAxis : uint8
{
	None,
	Horizontal,
	Vertical,
	Rotate180
};

UENUM(BlueprintType)
enum class EREMirrorInputType : uint8
{
	SymbolSequence,
	ClockHourSequence
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREMirrorCueDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	FName CueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	EREMirrorCueType CueType = EREMirrorCueType::Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (EditCondition = "CueType == EREMirrorCueType::Text", EditConditionHides))
	FText ActualText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (ClampMin = "1", ClampMax = "12", EditCondition = "CueType == EREMirrorCueType::ClockHour", EditConditionHides))
	int32 ActualClockHour = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	EREMirrorAxis MirrorAxis = EREMirrorAxis::Horizontal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	int32 DisplayOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	bool bShowOrderLabel = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue")
	FText DisplayPrefix;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREMirrorInputOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Input")
	FName InputId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Input")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Input", meta = (ClampMin = "1", ClampMax = "12"))
	int32 ClockHour = 12;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREMirrorSolutionStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Solution")
	FName ExpectedInputId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Solution", meta = (ClampMin = "1", ClampMax = "12"))
	int32 ExpectedClockHour = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Solution")
	FText StepDescription;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREMirrorInputResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	bool bAccepted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	bool bSolved = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	bool bReset = false;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	int32 CurrentStepIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	FText ResultMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Result")
	FText SubmittedInputText;
};
