#pragma once

#include "CoreMinimal.h"
#include "Interaction/REInteractable.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "REPuzzleInteractableActor.generated.h"

class UBoxComponent;
class UUserWidget;
class UWidgetComponent;
class APawn;

UCLASS(Abstract, Blueprintable)
class ROOMESCAPE_API AREPuzzleInteractableActor : public AREPuzzleActor, public IREInteractable
{
	GENERATED_BODY()

public:
	AREPuzzleInteractableActor();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FText InteractionPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FVector InteractionPromptRelativeLocation = FVector(0.0, 0.0, 140.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FVector2D InteractionPromptDrawSize = FVector2D(180.0, 60.0);

public:
	UFUNCTION(BlueprintPure, Category = "Puzzle|Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, Category = "Puzzle|Interaction")
	void ProcessServerInteract(AActor* Interactor);

protected:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanUseElement(AActor* Interactor) const;
	virtual void HandleInteract(AActor* Interactor);

	UFUNCTION()
	void OnInteractionPromptBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionPromptEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool ShouldShowPromptForActor(AActor* Actor) const;
	void SetInteractionPromptVisible(bool bVisible);
	void RefreshInteractionPromptText() const;
};
