// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RECommonActionWidget.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputSubsystem.h"

FSlateBrush URECommonActionWidget::GetIcon() const
{
	// 연결된 InputAction 유효성 확인
	if (IsValid(EnhancedInputAction) == true)
	{
		// Enhanced Input Subsystem 유효성 확인
		UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
		if (IsValid(EnhancedInputSubsystem) == true)
		{
			// 해당 InputAction에 할당된 전체 키 목록 얻기
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);
			if (BoundKeys.IsEmpty() == true)
			{
				return Super::GetIcon();
			}

			// CommonUI의 Input Subsystem 얻기
			UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
			if (IsValid(CommonInputSubsystem) == false)
			{
				return Super::GetIcon();
			}

			// 입력 키에 대한 아이콘 얻기
			FSlateBrush Brush;
			bool bIsBrushExisted = UCommonInputPlatformSettings::Get()->TryGetInputBrush(Brush, BoundKeys[0], CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName());
			if (bIsBrushExisted == true)
			{
				return Brush;
			}
		}
	}

	return Super::GetIcon();
}

UEnhancedInputLocalPlayerSubsystem* URECommonActionWidget::GetEnhancedInputSubsystem() const
{
	// InputAction을 사용하는 로컬 플레이어 얻기
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();

	// 특정 InputAction과 연결되어있는 실제 Widget Instance 얻기
	const UWidget* BoundWidget = DisplayedBindingHandle.GetBoundWidget();
	if (IsValid(BoundWidget) == true)
	{
		// Widget Instance가 유효하면 해당 Widget으로부터 LocalPlayer 얻기
		BoundWidget->GetOwningLocalPlayer();
	}

	// EnhancedInputLocalSubsystem 반환
	return IsValid(LocalPlayer) == true ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
}
