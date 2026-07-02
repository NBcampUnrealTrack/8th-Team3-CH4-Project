#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathMonitorWidget.generated.h"

class UButton;
class UTextBlock;
class ARETilePathMonitor;
class ARETilePathManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API URETilePathMonitorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_Question;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_Result;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_RevealedMoves;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_ClearMessage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_A;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_B;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_C;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_D;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_WaitingPlayer;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UTextBlock> TXT_GameStarted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	FText AllQuestionsSolvedText = FText::FromString(TEXT("CLEAR\n모든 문제를 맞혔습니다"));

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_A;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_B;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_C;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_D;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_Close;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Tile Path Monitor")
	TObjectPtr<UButton> BTN_Cancel;

	UPROPERTY(BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ARETilePathMonitor> MonitorActor;

	UPROPERTY(BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	FName CurrentQuestionId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Tile Path Monitor", meta = (AllowPrivateAccess = "true"))
	TArray<FRETilePathRevealedMove> CachedRevealedMoves;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void InitializeMonitor(ARETilePathMonitor* InMonitorActor);

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void SubmitAnswer(ERETilePathAnswerChoice SubmittedAnswer);

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void CloseMonitor();

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void HandleAnswerResult(bool bCorrect, const FRETilePathRevealedMove& RevealedMove, FName NextQuestionId);

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void RefreshQuestionView();

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void RefreshRevealedMovesView();

	UFUNCTION(BlueprintCallable, Category = "Tile Path Monitor")
	void RefreshSessionView();

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	ARETilePathMonitor* GetMonitorActor() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	ARETilePathManager* GetTilePathManager() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	bool GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	void GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Monitor")
	FName GetCurrentQuestionId() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Monitor")
	void ReceiveMonitorInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Monitor")
	void ReceiveAnswerResult(bool bCorrect, const FRETilePathRevealedMove& RevealedMove, FName NextQuestionId);

private:
	UFUNCTION()
	void HandleAnswerAClicked();

	UFUNCTION()
	void HandleAnswerBClicked();

	UFUNCTION()
	void HandleAnswerCClicked();

	UFUNCTION()
	void HandleAnswerDClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleSessionStateChanged(bool bStarted);

	UFUNCTION()
	void HandleParticipantsChanged();

	void BindButtons();
	void ApplyQuestionToWidgets(FName QuestionId, const FRETilePathQuestionRow& QuestionRow, bool bHasQuestion);
	void SetButtonText(UButton* Button, UTextBlock* ExplicitTextBlock, const FText& Text) const;
	void SetAnswerButtonsEnabled(bool bEnabled) const;
	void SetQuestionWidgetsVisible(bool bVisible) const;
	void SetWaitingWidgetsVisible(bool bVisible) const;
	void SetClearWidgetsVisible(bool bVisible) const;
	void SetRevealedMovesVisible(bool bVisible) const;
	bool AreAllQuestionsSolved() const;
	void BindManagerEvents();
	void UnbindManagerEvents();
	void CaptureInput();
	void RestoreInput();
	FText BuildRevealedMovesText() const;
};
