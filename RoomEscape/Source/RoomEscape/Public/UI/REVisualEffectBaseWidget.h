// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "REVisualEffectBaseWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetFadeInOutSignature);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREVisualEffectBaseWidget : public UCommonActivatableWidget
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
	// FadeIn Animation을 재생하는 함수
	UFUNCTION(BlueprintCallable)
	void FadeInWidget();

	// FadeOut Animation을 재생하는 함수
	// 재생 완료 시 Widget Deactivate 처리됨
	UFUNCTION(BlueprintCallable)
	void FadeOutWidget();

protected:
	// FadeIn Animation 재생 완료 시 호출되는 함수
	// OnWidgetFadeIn 이벤트 실행을 담당
	UFUNCTION()
	virtual void HandleFadeInAnimationFinished();

	// FadeOut Animation 재생 완료 시 호출되는 함수
	// OnWidgetFadeOut 이벤트 실행 및 Widget Deactivate 처리를 담당
	UFUNCTION()
	virtual void HandleFadeOutAnimationFinished();
};
