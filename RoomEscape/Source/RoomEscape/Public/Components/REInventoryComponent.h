// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetInitializableInterface.h"
#include "REInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryChangedSignature, const int32&, Index, const FPrimaryAssetId&, NewDataAssetID);

class UCommonActivatableWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOMESCAPE_API UREInventoryComponent : public UActorComponent, public IWidgetInitializableInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UREInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    // IWidgetInitializableInterface을(를) 통해 상속됨
    void InitWidget_Implementation() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(BlueprintAssignable)
    FInventoryChangedSignature OnInventoryChanged;

protected:
    // 인벤토리 Widget 클래스 (RootCanvas의 OverlayLayer에 Push되므로 ActivatableWidget 기반)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UCommonActivatableWidget> InventoryWidgetClass;

    // 인벤토리 크기
    // { RowCount, ColumnCount }
    UPROPERTY(EditAnywhere)
    FIntPoint InventoryCapacity;

    UPROPERTY(EditAnywhere)
    // 보관된 아이템이 위치한 인덱스를 보관하는 Map
    TMap<int32, FPrimaryAssetId> Container;

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItemToInventory(const FPrimaryAssetId& ItemDataAssetID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapItemIndex(const int32& OldIndex, const int32& NewIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FPrimaryAssetId RemoveItemFromInventory(const int32& DataIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FORCEINLINE FIntPoint GetInventoryCapacity() const { return InventoryCapacity; }

    FORCEINLINE const TMap<int32, FPrimaryAssetId>& GetInventoryContainer() const { return Container; }
};
