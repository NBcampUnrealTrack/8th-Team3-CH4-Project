// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REBoundActionButtonWidget.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"

void UREBoundActionButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// CommonUI에서 사용되는 InputMethod(Keyboard, Gamepad) 관련 Subsystem 유효성 확인
	UCommonInputSubsystem* InputSubSystem = GetInputSubsystem();
	if (IsValid(InputSubSystem) == true)
	{
		// InputMethod 변경 이벤트에 함수 추가 등록
		InputSubSystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::OnInputMethodChangedNative);

		// 현재 InputMethod로 Button Widget 초기화
		OnInputMethodChangedNative(InputSubSystem->GetCurrentInputType());
	}
}

void UREBoundActionButtonWidget::OnInputMethodChangedNative(ECommonInputType CurrentInputType)
{
	// 변경된 InputMethod에 따라 Button Style 변경
	switch (CurrentInputType)
	{
	case ECommonInputType::MouseAndKeyboard:
		SetStyle(MouseAndKeyboardButtonStyle);
		break;
	case ECommonInputType::Gamepad:
		SetStyle(GamepadButtonStyle);
		break;
	case ECommonInputType::Touch:
		SetStyle(TouchButtonStyle);
		break;
	case ECommonInputType::Count:
		SetStyle(CountButtonStyle);
		break;
	default:
		break;
	}
}
