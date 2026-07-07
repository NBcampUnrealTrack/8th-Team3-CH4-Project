#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorInputPanel.generated.h"

class UStaticMeshComponent;
class UREMirrorInputWidget;
class AREMirrorRoomManager;
class APlayerController;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREMirrorInputPanel : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREMirrorInputPanel();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PanelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREMirrorInputWidget> InputWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	FText InputPanelTitle = FText::FromString(TEXT("입력 패널"));

	UPROPERTY(Transient)
	TObjectPtr<UREMirrorInputWidget> ActiveInputWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SetMirrorManager(AREMirrorRoomManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Mirror Input")
	AREMirrorRoomManager* GetMirrorManager() const;

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void CloseInputWidget();

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SubmitInputId(const FName& InputId, AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SubmitClockHour(int32 ClockHour, AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void RequestInputReset(AActor* Interactor);

	UFUNCTION(Server, Reliable, Category = "Mirror Input")
	void ServerSubmitInputId(const FName& InputId, AActor* Interactor);

	UFUNCTION(Server, Reliable, Category = "Mirror Input")
	void ServerSubmitClockHour(int32 ClockHour, AActor* Interactor);

	UFUNCTION(Server, Reliable, Category = "Mirror Input")
	void ServerRequestInputReset(AActor* Interactor);

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION(Client, Reliable)
	void ClientOpenInputWidget(APlayerController* OwningPlayerController, TSubclassOf<UREMirrorInputWidget> InWidgetClass, const FText& InPanelTitle);

	UFUNCTION(Client, Reliable)
	void ClientReceiveInputResult(const FREMirrorInputResult& Result);

private:
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	void OpenInputWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREMirrorInputWidget> InWidgetClass, const FText& InPanelTitle);
	void DeliverInputResultToOwner(AActor* Interactor, const FREMirrorInputResult& Result);
};
