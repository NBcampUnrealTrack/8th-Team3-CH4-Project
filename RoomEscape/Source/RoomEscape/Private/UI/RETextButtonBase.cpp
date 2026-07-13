// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RETextButtonBase.h"
#include "Components/TextBlock.h"

void URETextButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsValid(TextBlock_ButtonText) == true)
	{
		TextBlock_ButtonText->SetText(ButtonText);
	}
}

void URETextButtonBase::SetButtonText(const FText& NewButtonText)
{
	if (IsValid(TextBlock_ButtonText) == true)
	{
		TextBlock_ButtonText->SetText(NewButtonText);
		ButtonText = NewButtonText;
	}
}
