// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "REInventoryEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREInventoryEntryWidget : public UCommonButtonBase, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_ItemIcon;

	TWeakObjectPtr<class UItemDataAsset> ItemDataAsset;

private:
	TSharedPtr<struct FStreamableHandle> AsyncLoadHandle;

public:
	void SetEntryDataAsset(const FPrimaryAssetId& NewDataAssetID);

	void OnLoadPrimaryAssetCompleted(FPrimaryAssetId NewDataAssetID);
};
