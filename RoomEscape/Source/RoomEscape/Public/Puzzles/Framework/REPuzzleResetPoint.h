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
};
