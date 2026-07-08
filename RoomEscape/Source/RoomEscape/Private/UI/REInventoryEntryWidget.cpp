// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REInventoryEntryWidget.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Item/ItemDataAsset.h"

void UREInventoryEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

}

void UREInventoryEntryWidget::NativeOnClicked()
{
    Super::NativeOnClicked();

    // 클릭 이벤트 연결 확인
    if (OnEntryClicked.IsBound() == false)
    {
        return;
    }

    // 이벤트 실행 및 반환값(FPrimaryAssetId) 유효성 검사
    FPrimaryAssetId DataAssetID = OnEntryClicked.Execute(EntryIndex);
    if (DataAssetID.IsValid() == false)
    {
        return;
    }
    // 아이템 스폰 절차 시작

    // 비동기 로드를 위한 에셋 매니저 참조
    UAssetManager& AssetManager = UAssetManager::Get();

    // 로드하려는 번들 설정
    const TArray<FName> Bundles = { TEXT("World") };

    // Callback 함수 설정
    FStreamableDelegate AsyncLoadCallback = FStreamableDelegate::CreateUObject(this, &ThisClass::OnLoadPrimaryAssetWorldBundleCompleted, DataAssetID);

    // 비동기 로드 시작
    AsyncLoadHandle = AssetManager.LoadPrimaryAsset(DataAssetID, Bundles, AsyncLoadCallback);
    return;
}

void UREInventoryEntryWidget::SetEntryDataAsset(const FPrimaryAssetId& DataAssetID)
{
    // 설정하려는 Asset ID가 유효한지 확인
    if (DataAssetID.IsValid() == false)
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
    FStreamableDelegate AsyncLoadCallback = FStreamableDelegate::CreateUObject(this, &ThisClass::OnLoadPrimaryAssetUIBundleCompleted, DataAssetID);

    // 비동기 로드 시작
    AsyncLoadHandle = AssetManager.LoadPrimaryAsset(DataAssetID, Bundles, AsyncLoadCallback);
    return;
}

void UREInventoryEntryWidget::OnLoadPrimaryAssetUIBundleCompleted(FPrimaryAssetId DataAssetID)
{
    if (DataAssetID.IsValid() == false)
    {
        return;
    }

    // 로드된 DataAsset 얻기 및 유효성 확인
    UItemDataAsset* ItemDataAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemDataAsset>(DataAssetID);
    if (IsValid(ItemDataAsset) == true)
    {
        // Widget의 Image 컴포넌트 설정
        Image_ItemIcon->SetBrushFromTexture(ItemDataAsset->Texture_Icon);
		Image_ItemIcon->SetVisibility(IsValid(ItemDataAsset->Texture_Icon) == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        return;
	}

    // 상정하지 않은 DataAsset의 경우 해당 Entry 비활성화 처리
	Image_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
    return;
}

void UREInventoryEntryWidget::OnLoadPrimaryAssetWorldBundleCompleted(FPrimaryAssetId DataAssetID)
{
    if (DataAssetID.IsValid() == false)
    {
        return;
    }

    // 로드된 DataAsset 얻기 및 유효성 확인
    UItemDataAsset* ItemDataAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemDataAsset>(DataAssetID);
    if (IsValid(ItemDataAsset) == true)
    {
        // 액터 스폰
        FActorSpawnParameters SpawnParams;
        SpawnParams.Instigator = GetOwningPlayerPawn();
        SpawnParams.Owner = GetOwningPlayerPawn();
        SpawnParams.Name = ItemDataAsset->ItemName;
        const FVector SpawnLocation = GetOwningPlayerPawn()->GetActorLocation();
        AActor* SpawnedItemActor = GetWorld()->SpawnActor(ItemDataAsset->ItemClass, &SpawnLocation, &FRotator::ZeroRotator, SpawnParams);
    }
    return;
}
