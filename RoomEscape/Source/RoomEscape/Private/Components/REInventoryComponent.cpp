// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/REInventoryComponent.h"
#include "Item/ItemDataAsset.h"
#include "UI/LocalWidgetManager.h"
#include "UI/InitializeUtilityInterface.h"
#include "UI/RERootCanvasWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

// Sets default values for this component's properties
UREInventoryComponent::UREInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	InventoryCapacity = FIntPoint(1, 1);
}


// Called when the game starts
void UREInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	IWidgetInitializableInterface::Execute_InitWidget(this);
}

void UREInventoryComponent::InitWidget_Implementation()
{
	// Local Client 확인
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false || Owner->HasLocalNetOwner() == false)
	{
		return;
	}

	// WidgetManager 유효성 확인
	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	// BeginPlay와 PawnClientRestart 양쪽에서 호출될 수 있으므로 중복 생성 방지
	if (IsValid(WidgetManager->FindWidget(FName("Inventory"))) == true)
	{
		return;
	}

	// RootCanvas 및 OverlayLayer 유효성 확인
	URERootCanvasWidget* RootCanvasWidget = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvasWidget) == false)
	{
		return;
	}

	UCommonActivatableWidgetStack* OverlayLayer = RootCanvasWidget->GetOverlayWidgetStack();
	if (IsValid(OverlayLayer) == false)
	{
		return;
	}

	// OverlayLayer에 Widget 생성(Push) 및 유효성 확인
	UCommonActivatableWidget* InventoryWidget = OverlayLayer->AddWidget<UCommonActivatableWidget>(InventoryWidgetClass);
	if (IsValid(InventoryWidget) == false)
	{
		return;
	}

	// 캐릭터의 토글 입력 등에서 FindWidget("Inventory")로 접근할 수 있도록 등록
	WidgetManager->AddWidgetInstance(FName("Inventory"), InventoryWidget);

	// Widget 초기화
	IInitializeUtilityInterface::Execute_InitializeWidgetByComponent(InventoryWidget, this);

	// 시작 시에는 숨김 상태, 토글 입력(I키)으로 표시
	WidgetManager->SetWidgetHiddenInGame(FName("Inventory"), true);
}

// Called every frame
void UREInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UREInventoryComponent::AddItemToInventory(const FPrimaryAssetId& ItemDataAssetID)
{
	if (ItemDataAssetID.IsValid() == false)
	{
		return false;
	}

	// 인벤토리가 가득 차있는지 확인
	int32 InventoryLength = InventoryCapacity.X * InventoryCapacity.Y;
	if (Container.Num() >= InventoryLength)
	{
		return false;
	}

	// 인벤토리의 각 슬롯마다 보관중인 데이터가 존재하는지 확인
	for (int32 i = 0; i < InventoryLength; ++i)
	{
		// 보관중인 데이터가 존재하지 않으면 해당 인덱스에 데이터 추가
		if (Container.Contains(i) == false)
		{
			Container.Add(i, ItemDataAssetID);

			// 인벤토리 정보 변화 이벤트 실행
			OnInventoryChanged.Broadcast(i, ItemDataAssetID);
			return true;
		}
	}

	return false;
}

bool UREInventoryComponent::SwapItemIndex(const int32& OldIndex, const int32& NewIndex)
{
	if (OldIndex < 0 || NewIndex < 0)
	{
		return false;
	}

	// 이전 인덱스에 위치한 DataAsset 복사 및 키(인덱스) 제거
	FPrimaryAssetId OldIndexDataAssetID;
	if (Container.RemoveAndCopyValue(OldIndex, OldIndexDataAssetID) == false)
	{
		// 이전 인덱스 위치에 데이터가 존재하지 않으면 함수 조기 종료
		return false;
	}

	// 새로운 인덱스에 위치한 DataAsset 복사 및 키(인덱스) 제거
	FPrimaryAssetId NewIndexDataAssetID;
	bool bIsSwap = Container.RemoveAndCopyValue(NewIndex, NewIndexDataAssetID);

	// 새로운 인덱스 위치에 데이터 이전
	Container.Add(NewIndex, OldIndexDataAssetID);

	// 새로운 인덱스 위치 데이터가 존재하였다면 이전 인덱스 위치에 데이터 이전
	if (bIsSwap == true)
	{
		Container.Add(OldIndex, NewIndexDataAssetID);
	}

	// 인벤토리 정보 변화 이벤트 실행
	OnInventoryChanged.Broadcast(OldIndex, NewIndexDataAssetID);
	OnInventoryChanged.Broadcast(NewIndex, OldIndexDataAssetID);

	return true;
}

FPrimaryAssetId UREInventoryComponent::RemoveItemFromInventory(const int32& DataIndex)
{
	if (DataIndex < 0)
	{
		return FPrimaryAssetId();
	}

	// DataAsset 복사 및 키(인덱스) 제거
	FPrimaryAssetId DataAssetID;
	if (Container.RemoveAndCopyValue(DataIndex, DataAssetID) == true)
	{
		// 인벤토리 정보 변화 이벤트 실행
		OnInventoryChanged.Broadcast(DataIndex, FPrimaryAssetId());

		// 제거된 키(인덱스)에 저장되어있던 값(데이터) 반환
		return DataAssetID;
	}
	return FPrimaryAssetId();
}
