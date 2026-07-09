// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RERootCanvasWidget.generated.h"

class UCommonActivatableWidgetStack;

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URERootCanvasWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> WidgetStack_Primary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> WidgetStack_Popup;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetPrimaryWidgetStack() const { return WidgetStack_Primary; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetPopupWidgetStack() const { return WidgetStack_Popup; }
};
