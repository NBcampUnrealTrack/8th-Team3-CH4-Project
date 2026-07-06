// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"
#include "Input/CommonUIInputTypes.h"
#include "Input/CommonBoundActionBar.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/REInventoryComponent.h"
#include "UI/REInventoryEntryWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void UInventoryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

void UInventoryWidget::InitializeWidgetByContextObject_Implementation(UObject* ContextObject)
{
}

void UInventoryWidget::InitializeWidgetByComponent_Implementation(UActorComponent* Component)
{
    // 인벤토리 Component 확인
    UREInventoryComponent* InventoryComponent = Cast<UREInventoryComponent>(Component);
    if (IsValid(InventoryComponent) == false)
    {
        return;
    }

    // 인벤토리 컨테이너 참조
    const TMap<int, FPrimaryAssetId>& InventoryContainer = InventoryComponent->GetInventoryContainer();

    // { Row, Column } 인벤토리 Capacity 얻기
    FIntPoint Capacity = InventoryComponent->GetInventoryCapacity();

    // Grid에 표시될 Entry(Slot) 개수 계산
    int32 InventoryLength = Capacity.X * Capacity.Y;

    // QuickSlot Layout 설정
    if (IsValid(UniformGrid_QuickSlot) == true)
    {
        // 현재 QuickSlot의 개수와 인벤토리의 Column 개수 비교
        int32 CurrentQuickSlotCount = UniformGrid_QuickSlot->GetChildrenCount();

        // QuickSlot Grid 조정
        for (int32 i = 0; i < Capacity.Y; ++i)
        {
            UREInventoryEntryWidget* InventoryEntryWidget = nullptr;

            // UniformGrid의 자식으로 존재하는 Entry(Slot) 인스턴스 얻기
            if (i < CurrentQuickSlotCount)
            {
                UWidget* ChildWidget = UniformGrid_QuickSlot->GetChildAt(i);
                InventoryEntryWidget = Cast<UREInventoryEntryWidget>(ChildWidget);
            }

            // Entry(Slot) 유효성 검사
            if (IsValid(InventoryEntryWidget) == false)
            {
                // 새로운 Widget 생성
                InventoryEntryWidget = CreateWidget<UREInventoryEntryWidget>(GetOwningPlayer(), InventoryEntryWidgetClass);

                // Grid의 자식으로 삽입
                UniformGrid_QuickSlot->InsertChildAt(i, InventoryEntryWidget);
            }

            // Entry Widget에 대하여 UniformGrid의 Row, Column 설정
            UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(InventoryEntryWidget->Slot);
            if (IsValid(GridSlot) == false)
            {
                continue;
            }
            GridSlot->SetRow(0);
            GridSlot->SetColumn(i);

            // 해당 인덱스에 할당된 아이템이 존재하면 Entry Widget에 Item 데이터 설정
            InventoryEntryWidget->SetEntryDataAsset(InventoryContainer.Contains(i) == true ? InventoryContainer[i] : FPrimaryAssetId());
        }
    }

    // 인벤토리 Layout 설정
    if (IsValid(UniformGrid_Inventory) == true)
    {
        // 현재 인벤토리 Grid에 존재하는 Entry(Slot) 개수 얻기
        int32 CurrentInventoryChildrenCount = UniformGrid_Inventory->GetChildrenCount();

        // 인벤토리 Grid 조정 (1줄(Row)는 QuickSlot에 사용한 것으로 간주)
        for (int32 i = Capacity.Y; i < InventoryLength; ++i)
        {
            UREInventoryEntryWidget* InventoryEntryWidget = nullptr;

            // UniformGrid의 자식으로 존재하는 Entry(Slot) 인스턴스 얻기
            if (i < CurrentInventoryChildrenCount)
            {
                UWidget* ChildWidget =UniformGrid_Inventory->GetChildAt(i);
                InventoryEntryWidget = Cast<UREInventoryEntryWidget>(ChildWidget);
            }

            // Entry(Slot) 유효성 검사
            if (IsValid(InventoryEntryWidget) == false)
            {
                // 새로운 Widget 생성
                InventoryEntryWidget = CreateWidget<UREInventoryEntryWidget>(GetOwningPlayer(), InventoryEntryWidgetClass);

                UniformGrid_Inventory->InsertChildAt(i, InventoryEntryWidget);
            }

            // Entry Widget에 대하여 UniformGrid의 Row, Column 설정
            UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(InventoryEntryWidget->Slot);
            if (IsValid(GridSlot) == false)
            {
                continue;
            }
            int32 Row = i / Capacity.Y;
            int32 Column = i % Capacity.Y;
            GridSlot->SetRow(Row - 1);
            GridSlot->SetColumn(Column);

            // 해당 인덱스에 할당된 아이템이 존재하면 Entry Widget에 Item 데이터 설정
            InventoryEntryWidget->SetEntryDataAsset(InventoryContainer.Contains(i) == true ? InventoryContainer[i] : FPrimaryAssetId());
        }
    }

    // 인벤토리 정보 변경에 대한 이벤트 연결
    InventoryComponent->OnInventoryChanged.AddDynamic(this, &ThisClass::OnInventoryChanged);
}

void UInventoryWidget::InitializeWidgetByActor_Implementation(AActor* Actor)
{
}

void UInventoryWidget::SetFocusedWidget(UCommonUserWidget* NewFocusedWidget)
{
    if (FocusedWidget == NewFocusedWidget)
    {
        return;
    }
}

void UInventoryWidget::OnInventoryChanged(const int32& Index, const FPrimaryAssetId& NewDataAssetID)
{
    if (Index < 0)
    {
        return;
    }

    // Index에 해당하는 Entry Widget Instance 얻기
    UWidget* ChildWidget;
    int32 ColumnCount = UniformGrid_QuickSlot->GetChildrenCount();
    if (Index < ColumnCount)
    {
        // 변화된 인덱스가 QuickSlot 영역에 해당
        ChildWidget = UniformGrid_QuickSlot->GetChildAt(Index);
    }
    else
    {
        // 변화된 인덱스가 Inventory 영역에 해당
        ChildWidget = UniformGrid_Inventory->GetChildAt(Index - ColumnCount);
    }

    // 참조된 Entry Widget Instance의 유효성 확인
    UREInventoryEntryWidget* EntryWidget = Cast<UREInventoryEntryWidget>(ChildWidget);
    if (IsValid(EntryWidget) == false)
    {
        return;
    }

    EntryWidget->SetEntryDataAsset(NewDataAssetID);
}
