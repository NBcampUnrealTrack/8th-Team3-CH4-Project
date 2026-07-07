#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorInputWidget.generated.h"

class UButton;
class UTextBlock;
class AREMirrorInputPanel;
class AREMirrorRoomManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API UREMirrorInputWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UTextBlock> TXT_InputTitle;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UTextBlock> TXT_CurrentInput;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UTextBlock> TXT_Result;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UTextBlock> TXT_ClearMessage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UTextBlock> TXT_InputOptions;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UButton> BTN_Close;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Mirror Input")
	TObjectPtr<UButton> BTN_Reset;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREMirrorInputPanel> InputPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	FText InputTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Mirror Input", meta = (AllowPrivateAccess = "true"))
	TArray<FREMirrorInputOption> CachedInputOptions;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;
	bool bLocalInputCompleted = false;
	FText LastSubmittedInputText;
	FText LastCompletionMessage;

public:
	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void InitializeInputPanel(AREMirrorInputPanel* InInputPanel, const FText& InInputTitle);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SubmitInputId(FName InputId);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SubmitClockHour(int32 ClockHour);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void SubmitInputOptionByIndex(int32 OptionIndex);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void RequestInputReset();

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void CloseInputPanel();

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void ApplyInputResult(const FREMirrorInputResult& Result);

	UFUNCTION(BlueprintCallable, Category = "Mirror Input")
	void RefreshInputView();

	UFUNCTION(BlueprintPure, Category = "Mirror Input")
	AREMirrorInputPanel* GetInputPanel() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Input")
	AREMirrorRoomManager* GetMirrorManager() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Mirror Input")
	void ReceiveInputPanelInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Mirror Input")
	void ReceiveInputResult(const FREMirrorInputResult& Result);

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleResetClicked();

	UFUNCTION()
	void HandleOption01Clicked();
	UFUNCTION()
	void HandleOption02Clicked();
	UFUNCTION()
	void HandleOption03Clicked();
	UFUNCTION()
	void HandleOption04Clicked();
	UFUNCTION()
	void HandleOption05Clicked();
	UFUNCTION()
	void HandleOption06Clicked();
	UFUNCTION()
	void HandleOption07Clicked();
	UFUNCTION()
	void HandleOption08Clicked();
	UFUNCTION()
	void HandleOption09Clicked();
	UFUNCTION()
	void HandleOption10Clicked();
	UFUNCTION()
	void HandleOption11Clicked();
	UFUNCTION()
	void HandleOption12Clicked();

	void FindOptionalWidgets();
	void BindOptionButtons();
	void BindOptionButton(int32 Index, UButton* Button);
	void RefreshOptionButtons();
	void RefreshCompletionView();
	void SetOptionButton(int32 Index, bool bVisible, const FText& OptionText);
	void SetTextByName(const FName& WidgetName, const FText& TextValue);
	void SetWidgetVisibilityByName(const FName& WidgetName, ESlateVisibility NewVisibility);
	UButton* FindButtonByOptionIndex(int32 Index) const;
	AActor* GetOwningInteractor() const;
	void CaptureInput();
	void RestoreInput();
};
