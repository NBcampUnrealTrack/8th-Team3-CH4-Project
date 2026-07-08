#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "RETilePathStartPanel.generated.h"

class UStaticMeshComponent;
class URETilePathWaitingWidget;
class ARETilePathManager;
class APlayerController;
class APawn;

UCLASS(Blueprintable)
class ROOMESCAPE_API ARETilePathStartPanel : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	ARETilePathStartPanel();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Start", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PanelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Start", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URETilePathWaitingWidget> WaitingWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<URETilePathWaitingWidget> ActiveWaitingWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Tile Path Start")
	void SetTilePathManager(ARETilePathManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Tile Path Start")
	ARETilePathManager* GetTilePathManager() const;

	UFUNCTION(BlueprintCallable, Category = "Tile Path Start")
	void CloseWaitingWidget();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Tile Path Start")
	void ServerCancelWalker();

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION(Client, Reliable)
	void ClientOpenWaitingWidget(APlayerController* OwningPlayerController);

	UFUNCTION(Client, Reliable)
	void ClientCloseWaitingWidget();

private:
	APawn* ResolvePawn(AActor* Actor) const;
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	void OpenWaitingWidgetLocal(APlayerController* PlayerController);
};
