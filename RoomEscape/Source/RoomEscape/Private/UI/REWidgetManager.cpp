// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REWidgetManager.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"

void UREWidgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UCommonActivatableWidgetContainerBase* UREWidgetManager::GetWidgetLayer(FGameplayTag LayerTag)
{
	// 찾는 Layer의 Tag 유효성 검사 및 Map에 존재하는지 검사
	if (LayerTag.IsValid() == false || LayerMap.Contains(LayerTag) == false)
	{
		return nullptr;
	}

	// Layer를 추적하는 Stack 얻기
	TArray<TWeakObjectPtr<UCommonActivatableWidgetContainerBase>>& LayerStack = LayerMap[LayerTag].LayerStack;

	for (int32 i = LayerStack.Num() - 1; i >= 0; --i)
	{
		// Stack의 최상단에서부터 순서대로 Layer 참조 포인터 얻기
		TWeakObjectPtr<UCommonActivatableWidgetContainerBase> Layer = LayerStack[i];

		// Layer 참조 유효성 확인
		if (Layer.IsValid() == false)
		{
			// 유효하지 않은 Layer를 Stack에서 삭제
			LayerStack.RemoveAt(i);
			continue;
		}

		// Layer를 소유한 UCommonActivatableWidget 객체 얻기
		UCommonActivatableWidget* OuterWidget = Layer->GetTypedOuter<UCommonActivatableWidget>();

		// Layer를 소유한 UCommonActivatableWidget 객체가 존재하면서 객체가 비활성 상태이면 다음 Layer 검색
		if (IsValid(OuterWidget) == true && OuterWidget->IsActivated() == false)
		{
			continue;
		}
		// Layer를 UCommonActivatableWidget 객체가 존재하지 않으면 해당 Layer는 항상 활성화 상태라고 가정

		// 함수 종료 전 메모리 정리
		LayerStack.Shrink();

		// Layer 반환
		return Layer.Get();
	}

	// 함수 종료 전 메모리 정리
	LayerStack.Shrink();

	// nullptr 반환
	return nullptr;
}

bool UREWidgetManager::PushWidgetLayer(UCommonActivatableWidgetContainerBase* Layer, FGameplayTag LayerTag)
{
	// Layer 및 Tag 유효성 검사
	if (IsValid(Layer) == false || LayerTag.IsValid() == false)
	{
		return false;
	}

	// Layer Tag 등록 여부 확인
	if (LayerMap.Contains(LayerTag) == false)
	{
		// LayerStack을 감싸는 Wraper 생성 및 해당 Layer 추가
		FLayerStack StackWraper;
		StackWraper.LayerStack.Push(Layer);

		// 새로운 LayerStack 등록
		LayerMap.Add(LayerTag, StackWraper);

		return true;
	}

	// Tag에 따른 Layer 얻기 및 유효성 검사
	UCommonActivatableWidgetContainerBase* PreviousLayer = GetWidgetLayer(LayerTag);
	if (IsValid(PreviousLayer) == false)
	{
		// Layer를 추적하는 Stack에 해당 Layer 추가
		LayerMap[LayerTag].LayerStack.Push(Layer);
		return true;
	}

	// 이전 Layer에 위치한 Widget을 새로 등록되는 Layer에 옮기기
	MoveWidgetsInLayer(PreviousLayer, Layer);

	// Layer를 추적하는 Stack에 해당 Layer 추가
	LayerMap[LayerTag].LayerStack.Push(Layer);

	return true;
}

bool UREWidgetManager::PopWidgetLayer(FGameplayTag LayerTag)
{
	// 찾는 Layer의 Tag 유효성 검사 및 Map에 존재하는지 검사
	if (LayerTag.IsValid() == false || LayerMap.Contains(LayerTag) == false)
	{
		return false;
	}

	// Layer를 추적하는 Stack얻기
	TArray<TWeakObjectPtr<UCommonActivatableWidgetContainerBase>>& LayerStack = LayerMap[LayerTag].LayerStack;

	// Stack이 비어있는지 검사
	if (LayerStack.IsEmpty() == true)
	{
		return false;
	}

	UCommonActivatableWidgetContainerBase* PreviousLayer = nullptr;
	for (int32 i = LayerStack.Num() - 1; i >= 0; --i)
	{
		// Stack의 최상단에서부터 순서대로 Layer 참조 포인터 얻기
		TWeakObjectPtr<UCommonActivatableWidgetContainerBase> Layer = LayerStack[i];

		// Layer 참조 유효성 확인
		if (Layer.IsValid() == false)
		{
			// 유효하지 않은 Layer를 Stack에서 삭제
			LayerStack.RemoveAt(i);
			continue;
		}

		// Layer를 소유한 UCommonActivatableWidget 객체 얻기
		UCommonActivatableWidget* OuterWidget = Layer->GetTypedOuter<UCommonActivatableWidget>();

		// Layer를 소유한 UCommonActivatableWidget 객체가 존재하면서 객체가 비활성 상태이면 다음 Layer 검색
		if (IsValid(OuterWidget) == true && OuterWidget->IsActivated() == false)
		{
			continue;
		}
		// Layer를 UCommonActivatableWidget 객체가 존재하지 않으면 해당 Layer는 항상 활성화 상태라고 가정

		// 이전에 제거한 Layer가 존재하지 않으면 현재 Index의 Layer 제거
		if (IsValid(PreviousLayer) == false)
		{
			PreviousLayer = LayerStack[i].Get();
			// Stack에서 제거
			LayerStack.RemoveAt(i);
		}
		else
		{
			// 이전 Layer에 존재하던 Widget 위치 변경
			MoveWidgetsInLayer(PreviousLayer, LayerStack[i].Get());
			return true;
		}
	}
	// 제거할 Layer가 존재하지 않음

	return PreviousLayer != nullptr;
}

bool UREWidgetManager::ClearWidgetLayer(FGameplayTag LayerTag)
{
	// 찾는 Layer의 Tag 유효성 검사 및 Map에 존재하는지 검사
	if (LayerTag.IsValid() == false || LayerMap.Contains(LayerTag) == false)
	{
		return false;
	}

	// Layer를 추적하는 Stack얻기
	TArray<TWeakObjectPtr<UCommonActivatableWidgetContainerBase>>& LayerStack = LayerMap[LayerTag].LayerStack;

	// Stack 초기화
	LayerStack.Reset();

	return true;
}

void UREWidgetManager::SetRootLayout(TSubclassOf<UUserWidget> RootWidgetClass)
{
	if (IsValid(RootWidgetClass) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("Invalid Widget Class"));
		return;
	}

	// Viewport Manager 얻기 및 유효성 확인
	UGameViewportSubsystem* ViewportManager =UGameViewportSubsystem::Get(GetWorld());
	if (IsValid(ViewportManager) == false)
	{
		return;
	}

	// 기존 Root Widget 제거
	if (IsValid(RootWidgetInstance) == true)
	{
		ViewportManager->RemoveWidget(RootWidgetInstance);
	}

	// Root Widget의 Instance 생성
	RootWidgetInstance = CreateWidget<UUserWidget>(GetWorld()->GetGameInstance(), RootWidgetClass);

	// Player의 Viewport에 UI 추가
	FGameViewportWidgetSlot RootWidgetSlot;
	RootWidgetSlot.bAutoRemoveOnWorldRemoved = false;
	ViewportManager->AddWidgetForPlayer(RootWidgetInstance, GetLocalPlayer(), RootWidgetSlot);
}

UCommonActivatableWidget* UREWidgetManager::AddWiget(const FREWidgetLayerData& WidgetLayerData)
{
	// 매개변수(데이터) 유효성 검사
	if (IsValid(WidgetLayerData.WidgetClass) == false || WidgetLayerData.LayerTag.IsValid() == false)
	{
		return nullptr;
	}

	// 목표 Layer 얻기 및 유효성 검사
	UCommonActivatableWidgetContainerBase* TargetLayer = GetWidgetLayer(WidgetLayerData.LayerTag);
	if (IsValid(TargetLayer) == false)
	{
		return nullptr;
	}

	// Widget 생성 및 생성된 Instance 얻기
	UCommonActivatableWidget* WidgetInstance = TargetLayer->AddWidget(WidgetLayerData.WidgetClass);

	// Instance 반환
	return WidgetInstance;
}

void UREWidgetManager::MoveWidgetsInLayer(UCommonActivatableWidgetContainerBase* PreviousLayer, UCommonActivatableWidgetContainerBase* CurrentLayer)
{
	if (IsValid(PreviousLayer) == false || IsValid(CurrentLayer) == false)
	{
		return;
	}

	// 이전 Layer에 위치한 Widget을 현재 등록되는 Layer에 옮기기
	TArray<UCommonActivatableWidget*> WidgetList = PreviousLayer->GetWidgetList();
	for (int32 i = 0; i < WidgetList.Num(); ++i)
	{
		// 이동되는 Widget 참조 및 유효성 검사
		UCommonActivatableWidget* WidgetInPreviousLayer = WidgetList[i];
		if (IsValid(WidgetInPreviousLayer) == false)
		{
			continue;
		}

		// 이전 Layer에서 Widget 제거
		PreviousLayer->RemoveWidget(*WidgetInPreviousLayer);

		// Widget을 현재 Layer에 추가
		CurrentLayer->AddWidgetInstance(*WidgetInPreviousLayer);
	}
}
