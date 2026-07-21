// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ChattingComponent.h"
#include "GameFramework/PlayerState.h"
#include "UI/LocalWidgetManager.h"
#include "UI/ChatBoxWidget.h"
#include "GameplayTags.h"
#include "EngineUtils.h"
#include "UI/RERootCanvasWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

// Sets default values for this component's properties
UChattingComponent::UChattingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UChattingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	// Owner를 PlayerController로 변환
	OwnerController = Cast<APlayerController>(GetOwner());

	// Widget 초기 설정 진행
	IWidgetInitializableInterface::Execute_InitWidget(this);
}

void UChattingComponent::InitWidget_Implementation()
{
	// Local Client 확인
	if (IsValid(OwnerController) == false || OwnerController->IsLocalController() == false)
	{
		return;
	}

	// WidgetManager 유효성 확인
	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	URERootCanvasWidget* RootCanvas = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvas) == false)
	{
		return;
	}

	UCommonActivatableWidgetStack* OverlayLayer = RootCanvas->GetOverlayWidgetStack();
	if (IsValid(OverlayLayer) == false)
	{
		return;
	}

	// Widget 생성 및 유효성 확인 (Overlay Layer에 생성)
	ChattingWidgetInstance = Cast<UChatBoxWidget>(OverlayLayer->AddWidget(ChattingWidgetClass));
	if (IsValid(ChattingWidgetInstance) == false)
	{
		return;
	}

	// Widget Manager에 Widget 등록
	WidgetManager->AddWidgetInstance(FName("Chatting"), ChattingWidgetInstance);

	// Widget 초기화
	IInitializeUtilityInterface::Execute_InitializeWidgetByComponent(ChattingWidgetInstance, this);
	
	// 채팅 전송 이벤트 연결
	ChattingWidgetInstance->OnMessageCommitted.AddDynamic(this, &UChattingComponent::ServerOnMessageCommitted);

	//// 채팅 전송 시 InputMode 변경 함수 연결
	//ChattingWidgetInstance->OnMessageCommitted.AddDynamic(this, &UChattingComponent::SetInputModeGameOnly);

}

void UChattingComponent::SetInputModeGameOnly()
{
	// Local Client 확인
	if (IsValid(OwnerController) == false || OwnerController->IsLocalController() == false)
	{
		return;
	}

	OwnerController->SetInputMode(FInputModeGameOnly());
	OwnerController->SetShowMouseCursor(false);
}

void UChattingComponent::ServerOnMessageCommitted_Implementation(const FGameplayTag& ChannelTag, const FString& Message)
{
	// Server 환경 확인 (ListenServer 또는 DedicatedServer)
	if (IsValid(OwnerController) == false || OwnerController->HasAuthority() == false)
	{
		return;
	}

	//// 채팅 채널이 유효하지 않거나 메시지가 없으면 채팅 불가
	//if (ChannelTag.IsValid() == false || Message.IsEmpty() == true)
	//{
	//	return;
	//}

	// 전송자 이름 얻기
	FString Sender = TEXT("");
	if (IsValid(OwnerController->PlayerState) == true)
	{
		Sender = OwnerController->PlayerState->GetPlayerName();
	}

	// 전체 PlayerController 목록에서 수신자 선별
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		// PlayerController의 유효성 확인
		if (IsValid(*It) == false)
		{
			continue;
		}

		// 채팅 관리 컴포넌트 검색
		UChattingComponent* ChattingComponent = It->FindComponentByClass<UChattingComponent>();
		if (IsValid(ChattingComponent) == false)
		{
			continue;
		}

		// 클라이언트에게 메시지 전달
		ChattingComponent->ClientOnMessageReceived(ChannelTag, Sender, Message);
	}
}

void UChattingComponent::ClientOnMessageReceived_Implementation(const FGameplayTag& ChannelTag, const FString& Sender, const FString& Message)
{
	// Local Client 확인
	if (IsValid(OwnerController) == false || OwnerController->IsLocalController() == false)
	{
		return;
	}

	// 메시지 수신 이벤트 실행
	OnMessageReceived.Broadcast(ChannelTag, Sender, Message);
}

