#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "REBombWireComponent.generated.h"

/**
 * Add this component directly to BP_BombDefusal and place it in the Blueprint
 * viewport. WireIndex is the stable, unique key that links exactly one
 * physical wire visual to one entry in the UREBombPatternData assigned to the
 * owning bomb actor. Leave it at INDEX_NONE only while authoring; the manager
 * rejects activation when an index is unset, missing from the DA, or duplicated.
 */
UCLASS(ClassGroup = (BombDefusal), Blueprintable, meta = (BlueprintSpawnableComponent))
class ROOMESCAPE_API UREBombWireComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UREBombWireComponent();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Wire", meta = (ClampMin = "-1", UIMin = "-1", AllowPrivateAccess = "true"))
	int32 WireIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	bool bHideWhenCut = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	bool bDisableCollisionWhenCut = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bomb Wire|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bRuntimeCut = false;

private:
	ECollisionEnabled::Type InitialCollisionEnabled = ECollisionEnabled::NoCollision;
	bool bInitialVisibility = true;
	bool bInitialStateCaptured = false;
	bool bHasAppliedRuntimeState = false;

public:
	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	int32 GetWireIndex() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	bool IsRuntimeCut() const;

	/** Applies optional mesh/material/style values from the DataAsset. */
	void ApplyDefinition(const FREBombWireDefinition& WireDefinition);

	/** Called by the owning integrated bomb actor whenever replicated state changes. */
	void ApplyCutState(bool bNewCut);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Wire")
	void ReceiveWireDefinitionApplied(const FREBombWireDefinition& WireDefinition);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Wire")
	void ReceiveWireCutChanged(bool bNewCut);

private:
	void CaptureInitialState();
	void RefreshVisualState();
};
