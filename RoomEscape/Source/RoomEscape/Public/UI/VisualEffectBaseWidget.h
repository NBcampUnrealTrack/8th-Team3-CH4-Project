// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "VisualEffectBaseWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetFadeInOutSignature);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UVisualEffectBaseWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	virtual void NativeOnActivated() override;

public:
	// Widget이 FadeIn 되었을 경우 실행되는 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnWidgetFadeInOutSignature OnWidgetFadeIn;

	// Widget이 FadeOut 되었을 경우 실행되는 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnWidgetFadeInOutSignature OnWidgetFadeOut;
	
protected:
	// FadeIn 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetAnim, AllowPrivateAccess = "true"), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation;

	// FadeOut 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetAnim, AllowPrivateAccess = "true"), Transient)
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;

	FWidgetAnimationDynamicEvent FadeInAnimationFinishedDelegate;

	FWidgetAnimationDynamicEvent FadeOutAnimationFinishedDelegate;

public:
	UFUNCTION()
	void HandleFadeInAnimationFinished();

	UFUNCTION()
	void HandleFadeOutAnimationFinished();
};
