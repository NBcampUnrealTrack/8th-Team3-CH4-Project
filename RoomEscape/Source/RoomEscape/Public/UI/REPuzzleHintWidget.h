#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REPuzzleHintWidget.generated.h"

class UButton;
class UTextBlock;
class AREPuzzleHintActor;

UCLASS(Blueprintable)
class ROOMESCAPE_API UREPuzzleHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Puzzle Hint")
	TObjectPtr<UTextBlock> TXT_HintTitle;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Puzzle Hint")
	TObjectPtr<UTextBlock> TXT_HintText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Puzzle Hint")
	TObjectPtr<UButton> BTN_Close;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREPuzzleHintActor> HintActor;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	FText HintTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle Hint", meta = (AllowPrivateAccess = "true"))
	FText HintText;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Puzzle Hint")
	void InitializeHint(AREPuzzleHintActor* InHintActor, const FText& InHintTitle, const FText& InHintText);

	UFUNCTION(BlueprintCallable, Category = "Puzzle Hint")
	void CloseHint();

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	AREPuzzleHintActor* GetHintActor() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	FText GetHintTitle() const;

	UFUNCTION(BlueprintPure, Category = "Puzzle Hint")
	FText GetHintText() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle Hint")
	void ReceiveHintInitialized();

private:
	UFUNCTION()
	void HandleCloseClicked();

	void ApplyHintText();
	void CaptureInput();
	void RestoreInput();
};
