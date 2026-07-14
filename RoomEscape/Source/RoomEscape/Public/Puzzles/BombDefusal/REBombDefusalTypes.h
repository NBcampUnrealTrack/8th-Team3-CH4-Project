#pragma once

#include "CoreMinimal.h"
#include "REBombDefusalTypes.generated.h"

class UMaterialInterface;
class UStaticMesh;

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
	ButtonState UMETA(DisplayName = "Button State")
};

/** UI entry type used by the integrated bomb-defusal widget. */
UENUM(BlueprintType)
enum class EREBombActionType : uint8
{
	Wire,
	Button
};

/**
 * Logical/UI definition for one wire.
 *
 * Layout is authored by placing a UREBombWireComponent inside BP_BombDefusal.
 * WireIndex is the stable key shared by the component, the WBP and SolutionSteps.
 */
USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREBombWireDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Identity", meta = (ClampMin = "-1", UIMin = "-1"))
	int32 WireIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Identity")
	EREBombWireColor WireColor = EREBombWireColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	FText WireLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	FLinearColor UIAccentColor = FLinearColor::Red;

	/** Disable this for a visual-only wire that should not create a WBP action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	bool bShowInWidget = true;

	/** Optional visual override for the matching component in BP_BombDefusal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UStaticMesh> WireMesh = nullptr;

	/** Optional material override for material slot 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UMaterialInterface> WireMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	bool bHideVisualWhenCut = true;
};

/**
 * Logical/UI definition for one device button.
 *
 * Layout and pressed movement are authored on a UREBombButtonComponent inside
 * BP_BombDefusal. ButtonId is the shared stable key.
 */
USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREBombButtonDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Identity")
	FName ButtonId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	FText ButtonLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	FLinearColor UIAccentColor = FLinearColor::White;

	/** Disable this for a visual-only button that should not create a WBP action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|UI")
	bool bShowInWidget = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|State")
	bool bInitialPressed = false;

	/** Optional visual override for the matching component in BP_BombDefusal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UStaticMesh> ButtonMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UMaterialInterface> ReleasedMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb|Visual")
	TObjectPtr<UMaterialInterface> PressedMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREBombStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	EREBombStepType StepType = EREBombStepType::CutWire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (EditCondition = "StepType == EREBombStepType::CutWire", EditConditionHides, ClampMin = "-1", UIMin = "-1"))
	int32 WireIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (EditCondition = "StepType == EREBombStepType::ButtonState", EditConditionHides))
	FName ButtonId = NAME_None;

	/** State expected immediately after this button is clicked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (EditCondition = "StepType == EREBombStepType::ButtonState", EditConditionHides))
	bool bRequiredButtonPressed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb")
	FText StepDescription;
};
