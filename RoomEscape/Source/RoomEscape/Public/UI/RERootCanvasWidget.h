// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Components/TimelineComponent.h"
#include "RERootCanvasWidget.generated.h"

class UCommonActivatableWidgetStack;

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URERootCanvasWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// MainMenu, HUD와 같이 특별한 조건이 없는 경우 항상 표시되어야 하는 UI를 위한 Layer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PrimaryLayer;

	// 게임 Level에 배치된 퍼즐과 같은 객체와 상호작용하는경우 표시되어야 하는 UI를 위한 Layer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameplayLayer;

	// Chatting, Inventory와 같이 다른 Layer와 함께 표시되어야 하는 UI를 위한 Layer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> OverlayLayer;

	// PauseMenu, Confirm UI와 같은 Popup UI를 위한 Layer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PopupLayer;

	// 화면 Fade out 효과같은 UI를 활용한 Visual Effect 처리를 담당하는 UI를 위한 Layer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> VisualEffectLayer;

public:
	// MainMenu, HUD와 같이 특별한 조건이 없는 경우 항상 표시되어야 하는 UI를 위한 Layer
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetPrimaryWidgetStack() const { return PrimaryLayer; }

	// 게임 Level에 배치된 퍼즐과 같은 객체와 상호작용하는경우 표시되어야 하는 UI를 위한 Layer
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetGameplayWidgetStack() const { return GameplayLayer; }

	// Chatting, Inventory와 같이 다른 Layer와 함께 표시되어야 하는 UI를 위한 Layer
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetOverlayWidgetStack() const { return OverlayLayer; }

	// PauseMenu, Confirm UI와 같은 Popup UI를 위한 Layer
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetPopupWidgetStack() const { return PopupLayer; }

	// 화면 Fade out 효과같은 UI를 활용한 Visual Effect 처리를 담당하는 UI를 위한 Layer
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCommonActivatableWidgetStack* GetVisualEffectWidgetStack() const { return VisualEffectLayer; }

protected:
	UFUNCTION(BlueprintCallable)
	void HideLayerWhenPopup();
};
