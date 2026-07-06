// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/InitializeUtilityInterface.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UInventoryWidget : public UCommonActivatableWidget, public IInitializeUtilityInterface
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

    virtual void NativeOnActivated() override;

    virtual void NativeOnDeactivated() override;

protected:
    // IInitializeUtilityInterface을(를) 통해 상속됨
    void InitializeWidgetByContextObject_Implementation(UObject* ContextObject) override;
    void InitializeWidgetByComponent_Implementation(UActorComponent* Component) override;
    void InitializeWidgetByActor_Implementation(AActor* Actor) override;

protected:
    // QuickSlot 영역을 담당하는 UniformGrid
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<class UUniformGridPanel> UniformGrid_QuickSlot;

    // 인벤토리 영역을 담당하는 UniformGrid
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<class UUniformGridPanel> UniformGrid_Inventory;

    UPROPERTY()
    TObjectPtr<UCommonUserWidget> FocusedWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UUserWidget> InventoryEntryWidgetClass;

protected:
    void SetFocusedWidget(UCommonUserWidget* NewFocusedWidget);

    UFUNCTION()
    void OnInventoryChanged(const int32& Index, const FPrimaryAssetId& NewDataAssetID);
};
