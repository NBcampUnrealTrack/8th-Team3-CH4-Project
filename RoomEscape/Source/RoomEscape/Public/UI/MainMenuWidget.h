// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "MainMenuWidget.generated.h"

class UCommonButtonBase;
class UCommonActivatableWidgetStack;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UCommonAnimatedSwitcher> Switcher_MenuButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> WidgetStack_Popup;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> WidgetStack_Primary;
	
	TSubclassOf<class UREPopupWidget> NotifyPopupWidgetClass;

protected:
};
