// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REPopupWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Spacer.h"
#include "UI/RETextButtonBase.h"

void UREPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UREPopupWidget::SetPopupContent(FText TextContent)
{
	if (IsValid(TextBlock_PopupContent) == false)
	{
		return;
	}

	TextBlock_PopupContent->SetText(TextContent);
}

URETextButtonBase* UREPopupWidget::AddPopupButton(FText Text_Button)
{
	if (IsValid(HorizontalBox_PopupButtons) == false || IsValid(ButtonClass) == false)
	{
		return nullptr;
	}

	// 기존에 동일한 버튼이 존재하면 해당 버튼을 반환
	if (Map_PopupButton.Contains(Text_Button.ToString()) == true)
	{
		return Map_PopupButton[Text_Button.ToString()];
	}

	// HorizontalBox에 다른 버튼이 존재하면 Spacer 추가
	if (HorizontalBox_PopupButtons->HasAnyChildren() == true)
	{
		AddSpacer(HorizontalBox_PopupButtons->GetChildrenCount() / 2);
	}

	// 버튼 생성 및 유효성 검사
	URETextButtonBase* NewButton = WidgetTree->ConstructWidget<URETextButtonBase>(ButtonClass);
	if (IsValid(NewButton) == false)
	{
		return nullptr;
	}

	// 버튼 문구 설정
	NewButton->SetButtonText(Text_Button);

	// Horizontal Box에 버튼 추가
	HorizontalBox_PopupButtons->AddChildToHorizontalBox(NewButton);

	// 생성된 버튼 목록에 NewButton 추가
	Map_PopupButton.Add(Text_Button.ToString(), NewButton);

	return NewButton;
}

void UREPopupWidget::AddPopupButton_Cancel()
{
	URETextButtonBase* Button_Cancel = AddPopupButton(FText::FromString(TEXT("Cancel")));
	Button_Cancel->OnClicked().RemoveAll(this);
	Button_Cancel->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
}

void UREPopupWidget::AddSpacer(const int32& SpacerPatternIndex)
{
	// Spacer가 추가될 Widget 유효성 검사
	if (IsValid(HorizontalBox_PopupButtons) == false)
	{
		return;
	}

	// Spacer 생성 및 유효성 검사
	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>();
	if (IsValid(Spacer) == false)
	{
		return;
	}

	// Spacer 크기 설정
	FVector2D SpaceSize = FVector2D(1, 1);
	if (EntrySpacingPatterns.IsEmpty() == false)
	{
		int32 PatternIndex = SpacerPatternIndex % EntrySpacingPatterns.Num();
		SpaceSize = EntrySpacingPatterns[PatternIndex];
	}
	Spacer->SetSize(SpaceSize);

	// HorizontalBox에 Spacer 추가
	UHorizontalBoxSlot* SpacerSlot = HorizontalBox_PopupButtons->AddChildToHorizontalBox(Spacer);
	SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
}
