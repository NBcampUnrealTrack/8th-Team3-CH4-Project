#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathWaitingWidget.generated.h"

class UTextBlock;
class UButton;
class APlayerController;
class ARETilePathManager;
class ARETilePathMonitor;
class ARETilePathStartPanel;

UCLASS(Blueprintable)
class ROOMESCAPE_API URETilePathWaitingWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeOnActivated() override;

	virtual void NativeOnDeactivated() override;

	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tile Path Waiting")
	void InitializeWaiting(AActor* InSourceActor, ARETilePathManager* InManager, ERETilePathParticipantRole InRole);

	UFUNCTION(BlueprintCallable, Category = "Tile Path Waiting")
	void CloseWaiting();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_WaitingPlayer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_Countdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_GameStart;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Cancel;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Waiting")
	void ReceiveCountdownChanged(int32 CountdownNumber, bool bGameStart);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Waiting")
	void ReceiveWaitingClosed(bool bStarted);

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> SourceActor;

	UPROPERTY(Transient)
	TObjectPtr<ARETilePathManager> TilePathManager;

	ERETilePathParticipantRole ParticipantRole = ERETilePathParticipantRole::None;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;
	bool bPulseActive = false;
	bool bPulseGameStart = false;
	float PulseElapsedSeconds = 0.0f;
	float PulseDurationSeconds = 0.35f;

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleSessionPhaseChanged(ERETilePathSessionPhase NewPhase);

	UFUNCTION()
	void HandleCountdownChanged(int32 CountdownNumber, bool bGameStart);

	UFUNCTION()
	void HandleParticipantsChanged();

	void BindManagerEvents();
	void UnbindManagerEvents();
	void RefreshWaitingView();
	void UpdateCountdownView(int32 InCountdownNumber, bool bGameStart);
	void StartCountdownPulse(bool bGameStart);
	UTextBlock* GetPulseTargetText() const;
	void CompleteWaiting(bool bStarted);
	void CaptureInput();
	void RestoreInput();
};
