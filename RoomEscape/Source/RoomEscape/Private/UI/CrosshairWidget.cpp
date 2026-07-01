// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CrosshairWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 기본 Texture를 사용하여 Crosshair Image 초기화
	InitializeCrosshairImageTexture();
}

void UCrosshairWidget::SetCrosshairImageTexture(TSoftObjectPtr<UTexture2D> Texture_Crosshair)
{
	// Image Component 및 설정하려는 Texture의 유효성 검사
	if (IsValid(Image_Crosshair) == false || Texture_Crosshair.IsNull() == true)
	{
		return;
	}
	// Image Component의 Texture 설정
	Image_Crosshair->SetBrushFromSoftTexture(Texture_Crosshair);
}

void UCrosshairWidget::SetCrosshairToCenter()
{
	if (IsValid(Image_Crosshair) == false)
	{
		return;
	}

	const FGeometry& CrosshairGeometry = Image_Crosshair->GetCachedGeometry();
	FVector2D CrosshairCenterAbsolutePosition = CrosshairGeometry.LocalToAbsolute(CrosshairGeometry.GetLocalSize() * 0.5f);

	const FGeometry& ViewportGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this);
	FVector2D ViewportCenterAbsolutePosition = ViewportGeometry.LocalToAbsolute(ViewportGeometry.GetLocalSize() * 0.5f);

	FVector2D TranslationDiff = ViewportCenterAbsolutePosition - CrosshairCenterAbsolutePosition;
	if (TranslationDiff.IsNearlyZero() == false)
	{
		FVector2D CurrentTranslation = GetRenderTransform().Translation;

		SetRenderTranslation(CurrentTranslation + TranslationDiff);
	}
	return;
}
