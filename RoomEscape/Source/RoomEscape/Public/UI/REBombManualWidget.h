#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REBombManualWidget.generated.h"

class UButton;
class UTextBlock;
class AREDefusalManual;

UCLASS(Blueprintable)
class ROOMESCAPE_API UREBombManualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Manual")
	TObjectPtr<UTextBlock> TXT_ManualTitle;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Manual")
	TObjectPtr<UTextBlock> TXT_ManualRules;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Manual")
	TObjectPtr<UButton> BTN_Close;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Manual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREDefusalManual> ManualActor;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Manual", meta = (AllowPrivateAccess = "true"))
	FText ManualTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Manual", meta = (AllowPrivateAccess = "true"))
	FText ManualText;

	TWeakObjectPtr<APlayerController> CapturedPlayerController;
	bool bCapturedInput = false;
	bool bPreviousMouseCursor = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Manual")
	void InitializeManual(AREDefusalManual* InManualActor, const FText& InManualTitle, const FText& InManualText);

	UFUNCTION(BlueprintCallable, Category = "Bomb Manual")
	void CloseManual();

	UFUNCTION(BlueprintPure, Category = "Bomb Manual")
	AREDefusalManual* GetManualActor() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Manual")
	void ReceiveManualInitialized();

private:
	UFUNCTION()
	void HandleCloseClicked();

	void ApplyManualText();
	void CaptureInput();
	void RestoreInput();
};
