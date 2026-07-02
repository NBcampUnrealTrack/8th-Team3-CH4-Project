#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "REPuzzleActor.generated.h"

class AREPuzzleManager;

UCLASS(Abstract, Blueprintable)
class ROOMESCAPE_API AREPuzzleActor : public AActor
{
	GENERATED_BODY()

public:
	AREPuzzleActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Puzzle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREPuzzleManager> PuzzleManager;

public:
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void SetPuzzleManager(AREPuzzleManager* InPuzzleManager);

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	AREPuzzleManager* GetPuzzleManager() const;
};
