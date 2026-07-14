// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REImageButtonBase.h"
#include "Components/Image.h"

void UREImageButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Image_Button) == true && Texture_Button.IsNull() == false)
	{
		Image_Button->SetBrushFromSoftTexture(Texture_Button);
	}
}

void UREImageButtonBase::SetButtonTexture(TSoftObjectPtr<UTexture2D> Texture)
{
	Texture_Button = Texture;
	if (IsValid(Image_Button) == true && Texture.IsNull() == false)
	{
		Image_Button->SetBrushFromSoftTexture(Texture);
	}
}
