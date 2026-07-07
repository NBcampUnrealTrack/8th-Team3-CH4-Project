#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REBombFeedbackWidget.generated.h"

class UButton;
class UTextBlock;
class APlayerController;

UCLASS(Blueprintable)
class ROOMESCAPE_API UREBombFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Feedback")
	TObjectPtr<UTextBlock> TXT_FeedbackTitle;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Feedback")
	TObjectPtr<UTextBlock> TXT_FeedbackMessage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Feedback")
	TObjectPtr<UButton> BTN_Close;

	FTimerHandle AutoCloseTimerHandle;
	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Feedback")
	void InitializeFeedback(AActor* SourceActor, bool bCorrect, const FText& FeedbackMessage, float DisplaySeconds);

	UFUNCTION(BlueprintCallable, Category = "Bomb Feedback")
	void CloseFeedback();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Feedback")
	void ReceiveFeedbackInitialized(AActor* SourceActor, bool bCorrect, const FText& FeedbackMessage);

private:
	UFUNCTION()
	void HandleCloseClicked();

	void ResolveWidgets();
	void CaptureInput();
	void RestoreInput();
};
