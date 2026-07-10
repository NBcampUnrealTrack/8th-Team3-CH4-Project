#pragma once

#include "CoreMinimal.h"
#include "Interaction/REInteractable.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "Puzzles/Framework/REPuzzleTypes.h"
#include "REPuzzleInteractableActor.generated.h"

class APawn;
class UBoxComponent;
class UTextRenderComponent;
class UUserWidget;
class UWidgetComponent;

UCLASS(Abstract, Blueprintable)
class ROOMESCAPE_API AREPuzzleInteractableActor : public AREPuzzleActor, public IREInteractable
{
	GENERATED_BODY()

public:
	AREPuzzleInteractableActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> InteractionPromptTextRender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FText InteractionPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FVector InteractionPromptRelativeLocation = FVector(0.0, 0.0, 140.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FVector2D InteractionPromptDrawSize = FVector2D(180.0, 60.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	bool bUseNativePromptFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float NativePromptWorldSize = 28.0f;

public:
	UFUNCTION(BlueprintPure, Category = "Puzzle|Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, Category = "Puzzle|Interaction")
	void ProcessServerInteract(AActor* Interactor);

protected:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanUseElement(AActor* Interactor) const;
	virtual void HandleInteract(AActor* Interactor);
	virtual void HandlePuzzleManagerChanged() override;

	UFUNCTION()
	void OnInteractionPromptBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionPromptEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnBoundPuzzleStateChanged(EREPuzzleState NewState);

private:
	TArray<TWeakObjectPtr<AActor>> PromptCandidateActors;
	TWeakObjectPtr<AREPuzzleManager> BoundPuzzleManager;

	bool ShouldTrackPromptActor(AActor* Actor) const;
	bool ShouldShowPromptForActor(AActor* Actor) const;
	void AddPromptCandidate(AActor* Actor);
	void RemovePromptCandidate(AActor* Actor);
	void RefreshInteractionPromptVisibility();
	void SetInteractionPromptVisible(bool bVisible);
	void RefreshInteractionPromptText() const;
	void BindPuzzleManagerStateChanged();
	void UnbindPuzzleManagerStateChanged();
};
