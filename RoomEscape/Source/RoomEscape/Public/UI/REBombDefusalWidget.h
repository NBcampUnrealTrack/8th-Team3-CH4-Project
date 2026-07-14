#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Puzzles/Framework/REPuzzleTypes.h"
#include "TimerManager.h"
#include "REBombDefusalWidget.generated.h"

class APlayerController;
class UButton;
class UPanelWidget;
class UTextBlock;
class UREBombDefusalActionEntryWidget;
class UREBombPatternData;
class AREBombDefusalManager;

/**
 * Main interactive bomb panel.
 *
 * Recommended WBP child names:
 *   TXT_Timer, TXT_Step, TXT_Feedback, PNL_Wires, PNL_Buttons, BTN_Close
 *
 * PNL_Wires/PNL_Buttons are populated automatically from the DataAsset.
 * ActionEntryWidgetClass is optional because a native fallback entry is used.
 * A custom entry WBP may expose BTN_Action, TXT_ActionLabel, TXT_ActionState
 * and IMG_Accent. A fully custom main WBP may instead bind fixed UMG buttons
 * to RequestCutWire / RequestToggleButton.
 */
UCLASS(Blueprintable)
class ROOMESCAPE_API UREBombDefusalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UTextBlock> TXT_Timer;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UTextBlock> TXT_Step;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UTextBlock> TXT_Feedback;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UPanelWidget> PNL_Wires;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UPanelWidget> PNL_Buttons;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Defusal")
	TObjectPtr<UButton> BTN_Close;

	/**
	 * True: PNL_Wires / PNL_Buttons are rebuilt from the DataAsset.
	 * False: designer-authored WBP buttons remain untouched and can call
	 * RequestCutWire / RequestToggleButton directly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Dynamic Entries", meta = (AllowPrivateAccess = "true"))
	bool bAutoBuildActionEntries = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Dynamic Entries", meta = (EditCondition = "bAutoBuildActionEntries", AllowPrivateAccess = "true"))
	TSubclassOf<UREBombDefusalActionEntryWidget> ActionEntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Close", meta = (AllowPrivateAccess = "true"))
	bool bCloseWhenSolved = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Close", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float SolvedCloseDelay = 0.75f;

	/** Prevents accidental double-clicks while the server is validating an action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Action", meta = (ClampMin = "0.1", Units = "s", AllowPrivateAccess = "true"))
	float ActionRequestTimeoutSeconds = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Defusal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREBombDefusalManager> BombManager;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UREBombDefusalActionEntryWidget>> ActionEntries;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	FTimerHandle SolvedCloseTimerHandle;
	FTimerHandle ActionRequestTimeoutTimerHandle;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;
	bool bActionRequestPending = false;
	bool bFailurePresentationActive = false;
	bool bFailureResultDelivered = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal")
	void InitializeBombDefusal(AREBombDefusalManager* InBombManager);

	/** Bind a UMG wire button to this and pass the WireIndex defined in the DA. */
	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal|Action")
	void RequestCutWire(int32 WireIndex);

	/** Bind a UMG device button to this and pass the ButtonId defined in the DA. */
	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal|Action")
	void RequestToggleButton(FName ButtonId);

	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal")
	void RefreshFromBombManager();

	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal")
	void RebuildActionEntries();

	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal")
	void CloseBombDefusal();

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	AREBombDefusalManager* GetBombManager() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	UREBombPatternData* GetPatternData() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Action")
	bool CanSubmitActions() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombDefusalInitialized(UREBombPatternData* InPatternData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombDisplayRefreshed(float RemainingTimeSeconds, int32 CurrentStepIndex, int32 TotalStepCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombActionResult(bool bCorrect, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombPatternChanged(UREBombPatternData* NewPatternData);

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleTimeChanged(float RemainingTimeSeconds, float TimeLimitSeconds);

	UFUNCTION()
	void HandleStepChanged(int32 CurrentStepIndex, int32 TotalStepCount);

	UFUNCTION()
	void HandleInputResult(AActor* SourceActor, bool bCorrect, FText ResultMessage);

	UFUNCTION()
	void HandleElementStatesChanged();

	UFUNCTION()
	void HandlePatternChanged(UREBombPatternData* NewPatternData);

	UFUNCTION()
	void HandlePuzzleStateChanged(EREPuzzleState NewState);

	UFUNCTION()
	void HandleBombExploded(AActor* SourceActor, FText ResultMessage);

	UFUNCTION()
	void HandleBombRuntimeReset();

	void ResolveWidgets();
	void BindCloseButton();
	void BindManagerEvents();
	void UnbindManagerEvents();
	void SetFailurePresentationActive(bool bActive);
	void RefreshEntryStates();
	void ApplyTimerText(float RemainingTimeSeconds);
	void ApplyStepText(int32 CurrentStepIndex, int32 TotalStepCount);
	void SetFeedbackText(const FText& FeedbackText, bool bVisible);
	void ScheduleSolvedClose();
	void ClearSolvedCloseTimer();
	void BeginActionRequest();
	void EndActionRequest();
	void HandleActionRequestTimeout();
	void CaptureInput();
	void RestoreInput();
};
