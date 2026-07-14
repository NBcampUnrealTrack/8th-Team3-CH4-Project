#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "REBombButtonComponent.generated.h"

class UMaterialInterface;

/**
 * Add this component directly to BP_BombDefusal and place it in the Blueprint
 * viewport. ButtonId is the stable, unique key that links exactly one
 * physical button visual to one entry in the UREBombPatternData assigned to
 * the owning bomb actor. The manager rejects activation when the ID is unset,
 * missing from the DA, or duplicated.
 */
UCLASS(ClassGroup = (BombDefusal), Blueprintable, meta = (BlueprintSpawnableComponent))
class ROOMESCAPE_API UREBombButtonComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UREBombButtonComponent();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	FName ButtonId = NAME_None;

	/** Offset from the component's authored Blueprint transform while pressed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	FTransform PressedTransformOffset = FTransform(FRotator::ZeroRotator, FVector(-3.0, 0.0, 0.0), FVector::OneVector);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	bool bApplyPressedTransform = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bomb Button|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bRuntimePressed = false;

private:
	FTransform ReleasedRelativeTransform = FTransform::Identity;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> ReleasedMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> PressedMaterial = nullptr;
	bool bReleasedStateCaptured = false;
	bool bHasAppliedRuntimeState = false;

public:
	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	FName GetButtonId() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	bool IsRuntimePressed() const;

	/** Applies optional mesh/material/style values from the DataAsset. */
	void ApplyDefinition(const FREBombButtonDefinition& ButtonDefinition);

	/** Called by the owning integrated bomb actor whenever replicated state changes. */
	void ApplyPressedState(bool bNewPressed);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Button")
	void ReceiveButtonDefinitionApplied(const FREBombButtonDefinition& ButtonDefinition);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Button")
	void ReceiveButtonPressedChanged(bool bNewPressed);

private:
	void CaptureReleasedTransform();
	void RefreshVisualState();
};
