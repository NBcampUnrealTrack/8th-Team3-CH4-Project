#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "REPuzzleResetPoint.generated.h"

class UArrowComponent;
class USceneComponent;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREPuzzleResetPoint : public AActor
{
	GENERATED_BODY()

public:
	AREPuzzleResetPoint();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle Reset Point", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle Reset Point", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Reset Point", meta = (AllowPrivateAccess = "true"))
	FName CheckpointId = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Reset Point", meta = (AllowPrivateAccess = "true"))
	int32 CheckpointOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Reset Point", meta = (AllowPrivateAccess = "true"))
	bool bUseAsCheckpoint = true;

public:
	UFUNCTION(BlueprintPure, Category = "Puzzle Reset Point")
	FName GetCheckpointId() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Reset Point")
	int32 GetCheckpointOrder() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Reset Point")
	bool CanUseAsCheckpoint() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Reset Point")
	FTransform GetCheckpointTransform() const;
};
