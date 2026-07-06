// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonBoundActionButton.h"
#include "REBoundActionButtonWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREBoundActionButtonWidget : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	// 키보드 & 마우스 입력 사용 시 사용되는 버튼 스타일
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> MouseAndKeyboardButtonStyle;

	// 게임 패드 사용 시 사용되는 버튼 스타일
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> GamepadButtonStyle;

	// 터치(Mobile) 사용 시 사용되는 버튼 스타일
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> TouchButtonStyle;

	// Count(?????) 사용 시 사용되는 버튼 스타일
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> CountButtonStyle;

protected:
	void OnInputMethodChangedNative(ECommonInputType CurrentInputType);
};
