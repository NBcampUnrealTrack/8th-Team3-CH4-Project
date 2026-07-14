// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "REInventoryEntryWidget.generated.h"

DECLARE_DELEGATE_RetVal_OneParam(FPrimaryAssetId, FInventoryEntryClickedSignature, const int32&);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREInventoryEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	// 인벤토리 슬롯을 클릭했을 경우 실행되는 이벤트
	// 인벤토리에서 해당 특정 인덱스의 아이템을 제거하는 함수에 연결
	FInventoryEntryClickedSignature OnEntryClicked;

protected:
	// 슬롯에 할당된 아이템의 아이콘을 표시하는 Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_ItemIcon;

	// 슬롯을 드래그 하는 동안 마우스를 따라서 보여지는 Widget의 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> DragVisualWidgetClass;

	UPROPERTY()
	int32 EntryIndex = -1;

private:
	TSharedPtr<struct FStreamableHandle> AsyncLoadHandle;

public:
	FORCEINLINE void SetEntryIndex(const int32& NewEntryIndex) { EntryIndex = NewEntryIndex; }

	void SetEntryDataAsset(const FPrimaryAssetId& DataAssetID);

	void OnLoadPrimaryAssetUIBundleCompleted(FPrimaryAssetId DataAssetID);

	void OnLoadPrimaryAssetWorldBundleCompleted(FPrimaryAssetId DataAssetID);

protected:
};
