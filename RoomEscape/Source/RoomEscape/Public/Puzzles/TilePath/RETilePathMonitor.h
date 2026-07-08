#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathMonitor.generated.h"

class UStaticMeshComponent;
class URETilePathMonitorWidget;
class URETilePathWaitingWidget;
class ARETilePathManager;
class APlayerController;

UCLASS(Blueprintable)
class ROOMESCAPE_API ARETilePathMonitor : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	ARETilePathMonitor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MonitorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URETilePathMonitorWidget> MonitorWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URETilePathWaitingWidget> WaitingWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<URETilePathMonitorWidget> ActiveMonitorWidget;

	UPROPERTY(Transient)
	TObjectPtr<URETilePathWaitingWidget> ActiveWaitingWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void SetTilePathManager(ARETilePathManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	ARETilePathManager* GetTilePathManager() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	bool GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	bool GetQuestionData(FName QuestionId, FRETilePathQuestionRow& OutQuestionRow) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	void GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	FName GetCurrentQuestionId() const;

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void CloseMonitorWidget();

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void CloseWaitingWidget();

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void OpenMonitorWidgetLocal(APlayerController* PlayerController);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Tile Path Monitor")
	void ServerSubmitAnswer(FName QuestionId, ERETilePathAnswerChoice SubmittedAnswer);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Tile Path Monitor")
	void ServerCancelGuide();

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION(Client, Reliable)
	void ClientOpenMonitorWidget(APlayerController* OwningPlayerController);

	UFUNCTION(Client, Reliable)
	void ClientOpenWaitingWidget(APlayerController* OwningPlayerController);

	UFUNCTION(Client, Reliable)
	void ClientReceiveAnswerResult(bool bCorrect, const FRETilePathRevealedMove& RevealedMove, FName NextQuestionId);

	UFUNCTION(Client, Reliable)
	void ClientCloseMonitorWidget();

	UFUNCTION(Client, Reliable)
	void ClientCloseWaitingWidget();

private:
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	void OpenWaitingWidgetLocal(APlayerController* PlayerController);
};
