#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "REBombDefusalActionEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UREBombDefusalWidget;

/**
 * Reusable WBP row/button created from one wire or button definition in the
 * Bomb Pattern DataAsset.
 *
 * Optional child widget names:
 *   BTN_Action, TXT_ActionLabel, TXT_ActionState, IMG_Accent
 *
 * A native button/text fallback is created when this C++ class is used without
 * a Blueprint child, so assigning an entry WBP is optional.
 */
UCLASS(Blueprintable)
class ROOMESCAPE_API UREBombDefusalActionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Action")
	TObjectPtr<UButton> BTN_Action;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Action")
	TObjectPtr<UTextBlock> TXT_ActionLabel;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Action")
	TObjectPtr<UTextBlock> TXT_ActionState;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Bomb Action")
	TObjectPtr<UImage> IMG_Accent;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREBombDefusalWidget> OwnerBombWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	EREBombActionType ActionType = EREBombActionType::Wire;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	int32 WireIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	FName ButtonId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	FText ActionLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action", meta = (AllowPrivateAccess = "true"))
	FLinearColor AccentColor = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "Bomb Action|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bCurrentState = false;

public:
	void InitializeWireAction(UREBombDefusalWidget* InOwnerWidget, const FREBombWireDefinition& WireDefinition);
	void InitializeButtonAction(UREBombDefusalWidget* InOwnerWidget, const FREBombButtonDefinition& ButtonDefinition);

	UFUNCTION(BlueprintCallable, Category = "Bomb Action")
	void RequestAction();

	UFUNCTION(BlueprintCallable, Category = "Bomb Action")
	void RefreshActionState();

	UFUNCTION(BlueprintPure, Category = "Bomb Action")
	EREBombActionType GetActionType() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Action")
	int32 GetWireIndex() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Action")
	FName GetButtonId() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Action")
	void ReceiveActionInitialized(EREBombActionType InActionType, const FText& InLabel, FLinearColor InAccentColor);

	/** bCurrentState means cut for wires and pressed for buttons. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Action")
	void ReceiveActionStateChanged(bool bNewCurrentState, bool bCanSubmit);

private:
	UFUNCTION()
	void HandleActionClicked();

	void EnsureNativeFallbackLayout();
	void ResolveWidgets();
	void BindButtonEvent();
	void ApplyVisualState(bool bCanSubmit);
};
