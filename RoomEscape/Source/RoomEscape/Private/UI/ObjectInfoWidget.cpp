// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ObjectInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UObjectInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UObjectInfoWidget::SetVisualObjectInfo(const TSoftObjectPtr<UTexture2D> Texture_ObjectIcon, const FText& ObjectName)
{
	// TextBlock 유효성 확인
	if (IsValid(TextBlock_ObjectName) == true && ObjectName.IsEmpty() == false)
	{
		// TextBlock 내용 설정
		TextBlock_ObjectName->SetText(ObjectName);
	}

	// Image 유효성 확인
	if (IsValid(Image_ObjectIcon) == true && Texture_ObjectIcon.IsNull() == false)
	{
		// Image에 사용되는 Texture 설정
		Image_ObjectIcon->SetBrushFromSoftTexture(Texture_ObjectIcon);
	}
}
