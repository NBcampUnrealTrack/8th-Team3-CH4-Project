#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REPuzzleHintActor.generated.h"

class UStaticMeshComponent;
class UREPuzzleHintWidget;
class APlayerController;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREPuzzleHintActor : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREPuzzleHintActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HintMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREPuzzleHintWidget> HintWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	FText HintTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true", MultiLine = "true"))
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Unlocked, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	bool bUnlocked = false;

	UPROPERTY(Transient)
	TObjectPtr<UREPuzzleHintWidget> ActiveHintWidget;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Puzzle Hint")
	void SetUnlocked(bool bNewUnlocked);

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	bool IsUnlocked() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	FText GetHintTitle() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	FText GetHintText() const;

	UFUNCTION(BlueprintCallable, Category = "Puzzle Hint")
	void CloseHintWidget();

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION()
	void OnRep_Unlocked();

	UFUNCTION(Client, Reliable)
	void ClientOpenHintWidget(APlayerController* OwningPlayerController, TSubclassOf<UREPuzzleHintWidget> InWidgetClass, const FText& InHintTitle, const FText& InHintText);

	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle Hint")
	void ReceiveUnlockedChanged(bool bNewUnlocked);

private:
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	APlayerController* ResolveLocalPlayerController(APlayerController* PreferredPlayerController) const;
	void OpenHintWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREPuzzleHintWidget> InWidgetClass, const FText& InHintTitle, const FText& InHintText);
};
