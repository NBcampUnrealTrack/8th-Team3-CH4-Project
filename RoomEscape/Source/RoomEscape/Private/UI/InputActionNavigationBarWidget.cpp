// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InputActionNavigationBarWidget.h"
#include "Components/HorizontalBox.h"
#include "UI/InputActionPromptWidget.h"

void UInputActionNavigationBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInputActionNavigationBarWidget::SetInputActionNavigationBar(const TArray<FInputActionPromptData>& InputActionPrompts)
{
	// 기존의 InputAction NavigationBar 초기화
	HorizontalBox_Root->ClearChildren();

	// 순서대로 사용자 입력에 대한 Widget 생성
	for (const FInputActionPromptData& InputActionPrompt : InputActionPrompts)
	{
		// 사용자 입력에 대한 Widget 생성
		UInputActionPromptWidget* IndicatorWidget = CreateWidget<UInputActionPromptWidget>(GetOwningPlayer(), InputActionPromptWidgetClass);
		if (IsValid(IndicatorWidget) == false)
		{
			continue;
		}

		// Widget 초기 설정 진행
		IndicatorWidget->InitializeIndicatorPrompt(InputActionPrompt);

		// 내비게이션 영역에 Widget 추가
		HorizontalBox_Root->AddChildToHorizontalBox(IndicatorWidget);
	}
}
