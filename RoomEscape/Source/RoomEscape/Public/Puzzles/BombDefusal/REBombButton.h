#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REBombButton.generated.h"

class UStaticMeshComponent;
class AREBombDefusalManager;
class APlayerState;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREBombButton : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREBombButton();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ButtonMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	FName ButtonId = TEXT("Main");

	UPROPERTY(ReplicatedUsing = OnRep_ButtonPressed, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	bool bPressed = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Bomb Button", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APlayerState> PressingPlayerState;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Button")
	void SetBombManager(AREBombDefusalManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	AREBombDefusalManager* GetBombManager() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	FName GetButtonId() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	bool IsPressed() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Button")
	APlayerState* GetPressingPlayerState() const;

	void ApplyServerPressedState(bool bNewPressed, APlayerState* InPressingPlayerState);

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION()
	void OnRep_ButtonPressed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Button")
	void ReceiveButtonPressedChanged(bool bNewPressed);
};
