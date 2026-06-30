// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CrosshairWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "UI/InputActionPromptWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//Slot->
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

}

void UCrosshairWidget::SetObjectInfo(UTexture2D* Texture_ObjectIcon, const FText& ObjectName)
{
	// 바라보고 있는 객체를 나타내는 UI의 아이콘 및 크기 설정
	if (IsValid(Image_ObjectIcon) == true && IsValid(Texture_ObjectIcon) == true)
	{
		Image_ObjectIcon->SetBrushFromTexture(Texture_ObjectIcon);
		Image_ObjectIcon->SetBrushSize({ 32, 32 });
	}

	// 바라보고 있는 객체 이름을 나타내는 TextBlock 내용 설정
	if (IsValid(TextBlock_ObjectName) == true && ObjectName.IsEmpty() == false)
	{
		TextBlock_ObjectName->SetText(ObjectName);
	}
}

void UCrosshairWidget::SetInputActionNavigationBar(const TArray<FInputActionPromptData>& InputActionPrompts)
{
	// 기존의 InputAction NavigationBar 초기화
	HorizontalBox_InputActionNavigation->ClearChildren();

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
		HorizontalBox_InputActionNavigation->AddChildToHorizontalBox(IndicatorWidget);
	}
}

void UCrosshairWidget::SetCrosshairToCenter(const FVector2D& CurrentSize)
{
	if (IsValid(Image_Crosshair) == false)
	{
		PreviousSize = CurrentSize;
		return;
	}

	PreviousSize = CurrentSize;
	return;
}
