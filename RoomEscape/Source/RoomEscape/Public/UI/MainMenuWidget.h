// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MainMenuWidget.generated.h"

class URETextButtonBase;

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
protected:
	// 게임 시작 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<URETextButtonBase> Button_Play;

	// 게임 종료 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<URETextButtonBase> Button_Quit;
};
