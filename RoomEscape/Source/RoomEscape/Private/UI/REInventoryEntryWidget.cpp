// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REInventoryEntryWidget.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDataAsset.h"

void UREInventoryEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    //Image_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
}

void UREInventoryEntryWidget::SetEntryDataAsset(const FPrimaryAssetId& NewDataAssetID)
{
    // 설정하려는 Asset ID가 유효한지 확인
    if (NewDataAssetID.IsValid() == false)
    {
        Image_ItemIcon->SetBrushFromTexture(nullptr);
        Image_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);

        return;
    }

    // 비동기 로드를 위한 에셋 매니저 참조
    UAssetManager& AssetManager = UAssetManager::Get();

    // 로드하려는 번들 설정
    const TArray<FName> Bundles = { TEXT("UI") };

    // Callback 함수 설정
    FStreamableDelegate AsyncLoadCallback = FStreamableDelegate::CreateUObject(this, &ThisClass::OnLoadPrimaryAssetCompleted, NewDataAssetID);

    // 비동기 로드 시작
    AsyncLoadHandle = AssetManager.LoadPrimaryAsset(NewDataAssetID, Bundles, AsyncLoadCallback);
}

void UREInventoryEntryWidget::OnLoadPrimaryAssetCompleted(FPrimaryAssetId NewDataAssetID)
{

    // 로드된 DataAsset 얻기 및 유효성 확인
    UItemDataAsset* ItemData = UAssetManager::Get().GetPrimaryAssetObject<UItemDataAsset>(NewDataAssetID);
    if (IsValid(ItemData) == true)
    {
        // Widget의 Image 컴포넌트 설정
        Image_ItemIcon->SetBrushFromTexture(ItemData->Texture_Icon);
		Image_ItemIcon->SetVisibility(IsValid(ItemData->Texture_Icon) == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        return;
	}

    // 상정하지 않은 DataAsset의 경우 해당 Entry 비활성화 처리
	Image_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
}
