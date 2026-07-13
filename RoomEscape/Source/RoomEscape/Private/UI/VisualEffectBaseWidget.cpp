// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/VisualEffectBaseWidget.h"
#include "Animation/WidgetAnimation.h"

void UVisualEffectBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(FadeInAnimation) == true)
	{
		FadeInAnimationFinishedDelegate.BindDynamic(this, &ThisClass::HandleFadeInAnimationFinished);
		BindToAnimationFinished(FadeInAnimation, FadeInAnimationFinishedDelegate);
	}

	if (IsValid(FadeOutAnimation) == true)
	{
		FadeOutAnimationFinishedDelegate.BindDynamic(this, &ThisClass::HandleFadeOutAnimationFinished);
		BindToAnimationFinished(FadeOutAnimation, FadeOutAnimationFinishedDelegate);
	}
}

void UVisualEffectBaseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	PlayAnimation(FadeInAnimation);
}

void UVisualEffectBaseWidget::HandleFadeInAnimationFinished()
{
	OnWidgetFadeIn.Broadcast();
}

void UVisualEffectBaseWidget::HandleFadeOutAnimationFinished()
{
	OnWidgetFadeOut.Broadcast();

	// Widget 비활성화
	DeactivateWidget();
}
