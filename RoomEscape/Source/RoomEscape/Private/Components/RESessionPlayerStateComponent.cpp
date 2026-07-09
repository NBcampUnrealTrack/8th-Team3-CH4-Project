// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RESessionPlayerStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UI/LocalWidgetManager.h"
#include "UI/RERootCanvasWidget.h"
#include "UI/RESessionRoomWidget.h"

// Sets default values for this component's properties
URESessionPlayerStateComponent::URESessionPlayerStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URESessionPlayerStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void URESessionPlayerStateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsPlayerReady);
}


// Called every frame
void URESessionPlayerStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URESessionPlayerStateComponent::InitWidget_Implementation()
{
	OwnerPlayerState = Cast<APlayerState>(GetOwner());
	if (IsValid(OwnerPlayerState) == false)
	{
		return;
	}

	if (OwnerPlayerState->HasLocalNetOwner() == false)
	{
		return;
	}

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	URERootCanvasWidget* RootCanvasWidget = WidgetManager->FindWidget<URERootCanvasWidget>(FName("RootCanvas"));
	if (IsValid(RootCanvasWidget) == false)
	{
		return;
	}

	UCommonActivatableWidgetStack* WidgetStack = RootCanvasWidget->GetPrimaryWidgetStack();
	if (IsValid(WidgetStack) == false)
	{
		return;
	}

	URESessionRoomWidget* SessionRoomWidget = WidgetStack->AddWidget<URESessionRoomWidget>(SessionRoomWidgetClass);
	if (IsValid(SessionRoomWidget) == false)
	{
		return;
	}

	// Ready 버튼 클릭 이벤트 연결
	SessionRoomWidget->OnButtonClicked_Ready.AddUniqueDynamic(this, &ThisClass::OnReadyButtonClicked);

	IInitializeUtilityInterface::Execute_InitializeWidgetByComponent(SessionRoomWidget, this);
}

bool URESessionPlayerStateComponent::CheckAllPlayerIsReady()
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		return false;
	}

	// 전체 PlayerController 목록에서 수신자 선별
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		// PlayerController 유효성 확인
		APlayerController* ClientController = *It;
		if (IsValid(ClientController) == false)
		{
			continue;
		}

		// PlayerState 얻기
		APlayerState* ClientState = ClientController->PlayerState;
		if (IsValid(ClientState) == false)
		{
			continue;
		}

		// 플레이어의 Ready 상태 관리 컴포넌트 얻기
		URESessionPlayerStateComponent* Component = ClientState->FindComponentByClass<URESessionPlayerStateComponent>();
		if (IsValid(Component) == false || Component == this)
		{
			continue;
		}

		// 플레이어의 Ready 상태 확인
		if (Component->IsPlayerReady() == false)
		{
			return false;
		}
	}
	return true;
}

void URESessionPlayerStateComponent::OnReadyButtonClicked()
{
	// Server
	if (GetOwnerRole() == ENetRole::ROLE_Authority)
	{
		// Session에 참가중인 전체 플레이어가 Ready 상태인지 확인
		bool bIsReadytoStart = CheckAllPlayerIsReady();
		if (bIsReadytoStart == false)
		{
			return;
		}

		// GameMap 로딩
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("#URESessionPlayerComponent - Need to Load Game Map"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("#URESessionPlayerComponent - Need to Load Game Map"));


		return;
	}

	// Client
	if (GetOwnerRole() == ENetRole::ROLE_AutonomousProxy || GetOwnerRole() == ENetRole::ROLE_SimulatedProxy)
	{
		// 서버에 Ready 상태 변경 요청
		ServerRequestChangeReadyState();
		return;
	}
}

void URESessionPlayerStateComponent::ServerRequestChangeReadyState_Implementation()
{
	if (GetOwnerRole() != ENetRole::ROLE_Authority)
	{
		return;
	}

	bIsPlayerReady = bIsPlayerReady == true ? false : true;

	// Listen 서버 환경을 고려하여 이벤트 실행
	OnRep_IsPlayerReady();
}

