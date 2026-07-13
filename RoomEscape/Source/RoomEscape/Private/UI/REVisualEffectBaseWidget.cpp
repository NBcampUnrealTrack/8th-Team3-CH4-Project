// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REVisualEffectBaseWidget.h"
#include "Animation/WidgetAnimation.h"

void UREVisualEffectBaseWidget::NativeConstruct()
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

void UREVisualEffectBaseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	PlayAnimation(FadeInAnimation);
}

void UREVisualEffectBaseWidget::FadeInWidget()
{
	if (IsValid(FadeInAnimation) == true)
	{
		PlayAnimation(FadeInAnimation);
	}
}

void UREVisualEffectBaseWidget::FadeOutWidget()
{
	if (IsValid(FadeOutAnimation) == true)
	{
		PlayAnimation(FadeOutAnimation);
	}
}

void UREVisualEffectBaseWidget::HandleFadeInAnimationFinished()
{
	OnWidgetFadeIn.Broadcast();
}

void UREVisualEffectBaseWidget::HandleFadeOutAnimationFinished()
{
	OnWidgetFadeOut.Broadcast();

	// Widget 비활성화
	DeactivateWidget();
}
