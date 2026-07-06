#pragma once

#include "CoreMinimal.h"
#include "REBombDefusalTypes.generated.h"

UENUM(BlueprintType)
enum class EREBombWireColor : uint8
{
	Red,
	Blue,
	Yellow,
	Green,
	White,
	Black
};

UENUM(BlueprintType)
enum class EREBombStepType : uint8
{
	CutWire,
	HoldButton
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREBombWireDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	int32 WireIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	EREBombWireColor WireColor = EREBombWireColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	FText WireLabel;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREBombStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	EREBombStepType StepType = EREBombStepType::CutWire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (EditCondition = "StepType == EREBombStepType::CutWire", EditConditionHides))
	int32 WireIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (EditCondition = "StepType == EREBombStepType::HoldButton", EditConditionHides))
	FName ButtonId = TEXT("Main");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0", EditCondition = "StepType == EREBombStepType::HoldButton", EditConditionHides))
	float RequiredHoldSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (ClampMin = "0.0", EditCondition = "StepType == EREBombStepType::HoldButton", EditConditionHides))
	float HoldTolerance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	FText StepDescription;
};
