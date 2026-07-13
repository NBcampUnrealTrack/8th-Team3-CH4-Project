// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UGameplayBaseWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	virtual void NativeOnDeactivated() override;

protected:
	// Content UI가 추가될 Widget Stack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout", meta = (BindWidget))
	TObjectPtr<class UCommonActivatableWidgetStack> WidgetStack_Contents;

	// 닫기 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout", meta = (BindWidget))
	TObjectPtr<class UCommonButtonBase> Button_Close;

public:
	// 임의 WidgetClass를 통해 생성된 UI를 Gameplay Widget의 Widget Stack에 추가하는 함수
	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidget* AddContentWidget(TSubclassOf<UCommonActivatableWidget> ContentWidgetClass);

	// 임의 UI의 Instance를 Gameplay Widget의 Widget Stack에 추가하는 함수
	UFUNCTION(BlueprintCallable)
	void AddContentWidgetInstance(UCommonActivatableWidget* ContentWidgetInstance);

	// 현재 Gameplay Widget의 Widget Stack에 표시되고 있는 UI의 Instance를 얻는 함수 
	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidget* GetContentWidgetInstance() const;

protected:
	// 닫기 버튼 클릭 시 실행되는 함수
	// Widget Stack에 표시되는 Widget이 없는 경우 현재 UI를 닫는다.
	UFUNCTION()
	void OnCloseButtonClicked();
};
