// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RERootCanvasWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UI/LocalWidgetManager.h"

void URERootCanvasWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 각 Layer를 WidgetManager에 등록
	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == true)
	{
		if (IsValid(PrimaryLayer) == true)
		{
			WidgetManager->AddWidgetInstance(FName("PrimaryLayer"), PrimaryLayer);
		}
		if (IsValid(GameplayLayer) == true)
		{
			WidgetManager->AddWidgetInstance(FName("GameplayLayer"), GameplayLayer);
		}
		if (IsValid(OverlayLayer) == true)
		{
			WidgetManager->AddWidgetInstance(FName("OverlayLayer"), OverlayLayer);
		}
		if (IsValid(PopupLayer) == true)
		{
			WidgetManager->AddWidgetInstance(FName("PopupLayer"), PopupLayer);
		}
		if (IsValid(VisualEffectLayer) == true)
		{
			WidgetManager->AddWidgetInstance(FName("VisualEffectLayer"), VisualEffectLayer);
		}
	}
}

void URERootCanvasWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Popup Widget의 존재에 따라서 Primary와 Gameplay Layer의 Visisbility 변경
	HideLayerWhenPopup();
}

void URERootCanvasWidget::HideLayerWhenPopup()
{
	if (IsValid(PopupLayer) == false)
	{
		return;
	}
	
	// Popup Layer에 활성화 되어있는 Widget 얻기
	UCommonActivatableWidget* PopupWidget = PopupLayer->GetActiveWidget();

	// Popup Widget이 유효하면 Primary Layer를 Collapsed 처리
	if (IsValid(PrimaryLayer) == true)
	{
		PrimaryLayer->SetVisibility(IsValid(PopupWidget) == true ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	// Popup Widget이 유효하면 Gameplay Layer를 Collapsed 처리
	if (IsValid(GameplayLayer) == true)
	{
		GameplayLayer->SetVisibility(IsValid(PopupWidget) == true ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
}
