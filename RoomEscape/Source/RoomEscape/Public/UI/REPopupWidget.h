// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "REPopupWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREPopupWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	// Session의 이름을 입력받는 EditableTextBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Layout")
	TSubclassOf<class URETextButtonBase> ButtonClass;

	// Popup의 내용을 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<class UTextBlock> TextBlock_PopupContent;

	// Popup창에서 사용자가 클릭할 버튼을 담는 HorizontalBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<class UHorizontalBox> HorizontalBox_PopupButtons;

	// Popup창에 다수의 버튼이 표시되는 경우, 해당 버튼들 사이에 추가되는 Spacer의 크기
	// { (Horizontal, Vertical), ... }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Layout")
	TArray<FVector2D> EntrySpacingPatterns;

public:
	void SetPopupContent(FText TextContent);

	UFUNCTION(BlueprintCallable)
	class URETextButtonBase* AddPopupButton(FText Text_Button);

protected:
	UFUNCTION(BlueprintCallable)
	void AddSpacer(const int32& SpacerPatternIndex);
};
