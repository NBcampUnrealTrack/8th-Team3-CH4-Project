// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REPopupWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
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

	return NewButton;
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
	HorizontalBox_PopupButtons->AddChildToHorizontalBox(Spacer);
}
