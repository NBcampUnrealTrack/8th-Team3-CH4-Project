// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameplayBaseWidget.h"
#include "Components/Overlay.h"
#include "CommonButtonBase.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UGameplayBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 등록
	if (IsValid(Button_Close) == true)
	{
		Button_Close->OnClicked().RemoveAll(this);
		Button_Close->OnClicked().AddUObject(this, &ThisClass::OnCloseButtonClicked);
	}
}

void UGameplayBaseWidget::NativeOnDeactivated()
{
	// Contents 내용 전체 제거
	WidgetStack_Contents->ClearWidgets();

	Super::NativeOnDeactivated();
}

UCommonActivatableWidget* UGameplayBaseWidget::AddContentWidget(TSubclassOf<UCommonActivatableWidget> ContentWidgetClass)
{
	if (IsValid(WidgetStack_Contents) == false)
	{
		return nullptr;
	}

	return IsValid(ContentWidgetClass) == true ? WidgetStack_Contents->AddWidget(ContentWidgetClass) : nullptr;
}

void UGameplayBaseWidget::AddContentWidgetInstance(UCommonActivatableWidget* ContentWidgetInstance)
{
	if (IsValid(WidgetStack_Contents) == true && IsValid(ContentWidgetInstance) == true)
	{
		WidgetStack_Contents->AddWidgetInstance(*ContentWidgetInstance);
	}
	return;
}

UCommonActivatableWidget* UGameplayBaseWidget::GetContentWidgetInstance() const
{
	if (IsValid(WidgetStack_Contents) == false)
	{
		return nullptr;
	}

	// Stack에 활성화 되어있는 Widget 얻기
	UCommonActivatableWidget* ActivatedContentWidget = WidgetStack_Contents->GetActiveWidget();
	return IsValid(ActivatedContentWidget) == true ? ActivatedContentWidget : nullptr;
}

void UGameplayBaseWidget::OnCloseButtonClicked()
{
	if (IsValid(WidgetStack_Contents) == true)
	{
		// Content로 활성화 되어있는 Widget이 존재하면 해당 Widget 비활성화(닫기)
		UCommonActivatableWidget* ActivatedContentWidget = WidgetStack_Contents->GetActiveWidget();
		if (IsValid(ActivatedContentWidget) == true)
		{
			ActivatedContentWidget->DeactivateWidget();
		}

		if (WidgetStack_Contents->GetNumWidgets() > 1)
		{
			return;
		}
	}

	// Content로 활성화 되어있는 Widget이 존재하지 않으며 현재 Widget(GameplayWidget) 비활성화(닫기)
	DeactivateWidget();
}
