#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "REDialLockWidget.generated.h"

class UButton;
class UTextBlock;
class USoundBase;
class AREDialLockDevice;
class ARELockAndGlowClueManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API UREDialLockWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UTextBlock> TXT_Digit0;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UTextBlock> TXT_Digit1;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UTextBlock> TXT_Digit2;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UTextBlock> TXT_Digit3;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UTextBlock> TXT_Feedback;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit0Up;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit1Up;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit2Up;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit3Up;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit0Down;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit1Down;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit2Down;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Digit3Down;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Submit;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Dial Lock")
	TObjectPtr<UButton> BTN_Close;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dial Lock|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DigitUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dial Lock|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DigitDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dial Lock|Feedback", meta = (AllowPrivateAccess = "true"))
	bool bCloseAfterCorrectResult = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dial Lock|Feedback", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float CorrectResultCloseDelay = 0.75f;

	UPROPERTY(BlueprintReadOnly, Category = "Dial Lock", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREDialLockDevice> LockDevice;

	UPROPERTY(BlueprintReadOnly, Category = "Dial Lock", meta = (AllowPrivateAccess = "true"))
	TArray<int32> CachedDigits;

	UPROPERTY(BlueprintReadOnly, Category = "Dial Lock|Feedback", meta = (AllowPrivateAccess = "true"))
	FText LastResultMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Dial Lock|Feedback", meta = (AllowPrivateAccess = "true"))
	bool bLastResultCorrect = false;

	TWeakObjectPtr<ARELockAndGlowClueManager> BoundLockManager;
	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	FTimerHandle CorrectResultCloseTimerHandle;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;
	bool bReceivedResultForCurrentOpen = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void InitializeDialLock(AREDialLockDevice* InLockDevice, const TArray<int32>& InitialDigits);

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void RequestIncrementDigit(int32 DigitIndex);

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void RequestDecrementDigit(int32 DigitIndex);

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void RequestSetDigit(int32 DigitIndex, int32 DigitValue);

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void RequestSubmitCode();

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void CloseDialLock();

	UFUNCTION(BlueprintPure, Category = "Dial Lock")
	AREDialLockDevice* GetLockDevice() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock")
	void ReceiveDialLockInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock")
	void ReceiveDigitsChanged(const TArray<int32>& NewDigits);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock|Feedback")
	void ReceiveSubmitResult(bool bCorrect, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock|Sound")
	void ReceiveDigitStepRequested(int32 DigitIndex, bool bIncrementing);

private:
	UFUNCTION()
	void HandleDigitsChanged(const TArray<int32>& NewDigits);

	UFUNCTION()
	void HandleUnlockedChanged(bool bNewUnlocked);

	UFUNCTION()
	void HandleInputResult(AActor* SourceActor, bool bCorrect, FText ResultMessage);

	UFUNCTION()
	void HandleDigit0UpClicked();

	UFUNCTION()
	void HandleDigit1UpClicked();

	UFUNCTION()
	void HandleDigit2UpClicked();

	UFUNCTION()
	void HandleDigit3UpClicked();

	UFUNCTION()
	void HandleDigit0DownClicked();

	UFUNCTION()
	void HandleDigit1DownClicked();

	UFUNCTION()
	void HandleDigit2DownClicked();

	UFUNCTION()
	void HandleDigit3DownClicked();

	UFUNCTION()
	void HandleSubmitClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void ResolveWidgets();
	void BindButtonEvents();
	void BindDeviceEvents();
	void UnbindDeviceEvents();
	void ApplyDigitsToText();
	void ClearFeedbackText();
	void DisplaySubmitResult(bool bCorrect, const FText& ResultMessage);
	void ScheduleCloseAfterCorrectResult();
	void ClearCloseTimer();
	void PlayDigitStepSound(int32 DigitIndex, bool bIncrementing);
	void PlayUISound(USoundBase* Sound) const;
	void CaptureInput();
	void RestoreInput();
};
