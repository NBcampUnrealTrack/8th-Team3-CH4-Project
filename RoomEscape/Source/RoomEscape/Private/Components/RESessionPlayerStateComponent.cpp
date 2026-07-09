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
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void URESessionPlayerStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerPlayerState = Cast<APlayerState>(GetOwner());
}

void URESessionPlayerStateComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsInSessionRoom);
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

	URERootCanvasWidget* RootCanvasWidget = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvasWidget) == false)
	{
		return;
	}

	UCommonActivatableWidgetStack* PrimaryLayer = RootCanvasWidget->GetPrimaryWidgetStack();
	if (IsValid(PrimaryLayer) == false)
	{
		return;
	}

	SessionRoomWidgetInstance = PrimaryLayer->AddWidget<URESessionRoomWidget>(SessionRoomWidgetClass);
	if (IsValid(SessionRoomWidgetInstance) == true)
	{
		// Ready 버튼 클릭 이벤트 연결
		SessionRoomWidgetInstance->OnButtonClicked_Ready.AddUniqueDynamic(this, &ThisClass::OnReadyButtonClicked);
	}
	return;
}

void URESessionPlayerStateComponent::JoinSessionRoom()
{
	// Session Room UI 생성
	IWidgetInitializableInterface::Execute_InitWidget(this);

	// 서버에 플레이어가 Session Room에 참여하였음을 알림
	ServerBroadcastPlayerInSessionRoom(true);
}

void URESessionPlayerStateComponent::LeaveSessionRoom()
{
	// 서버에 플레이어가 Session Room에 퇴장하였음을 알림
	ServerBroadcastPlayerInSessionRoom(true);

	// Session Room UI 제거
	if (IsValid(SessionRoomWidgetInstance) == true)
	{
		SessionRoomWidgetInstance->DeactivateWidget();
	}
}

void URESessionPlayerStateComponent::ServerBroadcastPlayerInSessionRoom_Implementation(bool bIsJoin)
{
	// 서버에서만 실행되도록 확인
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasAuthority() == false)
	{
		return;
	}

	// Session Room에 참여하였는지를 판단하는 플래그 값 변경
	bIsInSessionRoom = bIsJoin;

	// World에 존재하는 모든 PlayerState 중에서 Session에 참여되어있는 모든 Client에게 전파
	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
	{
		// PlayerState의 유효성 검사
		APlayerState* ClientState = *It;
		if (IsValid(ClientState) == false)
		{
			continue;
		}

		// Session Room 참여 상태를 관리하는 Component 얻기
		URESessionPlayerStateComponent* SessionComponent = ClientState->FindComponentByClass<URESessionPlayerStateComponent>();
		if (IsValid(SessionComponent) == false)
		{
			continue;
		}

		// 클라이언트가 현재 SessionRoom에 참여한 상태인지 확인
		if (SessionComponent->bIsInSessionRoom == true)
		{
			// 클라이언트에게 자신(플레이어)의 SessionRoom에대한 상태가 변경되었음을 전파
			ClientBroadcastPlayerInSessionRoom(OwnerPlayerState, bIsJoin);
		}
	}
}

void URESessionPlayerStateComponent::ClientBroadcastPlayerInSessionRoom_Implementation(APlayerState* PlayerState, bool bIsJoin)
{
	// LocalPlayerState에서만 실행되도록 확인
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasLocalNetOwner() == false)
	{
		return;
	}

	// 생성된 Session Room UI 확인
	if (IsValid(SessionRoomWidgetInstance) == false)
	{
		return;
	}

	// 참여한 Player를 UI에 추가 또는 제거
	if (bIsJoin == true)
	{
		// 플레이어 추가
		SessionRoomWidgetInstance->AddJoinedPlayer(PlayerState);
	}
	else
	{
		// 플레이어 제거
		SessionRoomWidgetInstance->RemoveLeavePlayer(PlayerState);
	}
}

void URESessionPlayerStateComponent::OnReadyButtonClicked()
{
	// 컴포넌트를 소유한 PlayerState 확인
	if (IsValid(OwnerPlayerState) == false)
	{
		return;
	}

	// Listen Server의 경우 SessionRoom에 참여한 전체 PlayerState의 Ready 상태 확인
	if (OwnerPlayerState->HasAuthority() == false)
	{
		// Session Room에 참여한 전체 플레이어의 Ready 상태 검사
		bool bAllPlayerIsReady = CheckAllPlayerIsReady();

		// 참여한 전체 플레이어가 Ready 상태일 경우 게임 맵 로드
		if (bAllPlayerIsReady == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("# URESessionPlayerStateComponent - Need to open Game Map"));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("# URESessionPlayerStateComponent - Need to open Game Map"));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("# URESessionPlayerStateComponent - Need to open Game Map"));
			return;
		}
	}
	// Client의 경우 서버에 Ready 상태 변경 요청 전송
	else
	{
		// 서버에게 플레이어의 Ready 상태 변경 요청
		ServerRequestChangeReadyState(!bIsPlayerReady);
	}
}

bool URESessionPlayerStateComponent::CheckAllPlayerIsReady()
{
	// World에 존재하는 모든 PlayerState 중에서 Session에 참여되어있는 모든 PlayerState 대상 검사
	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
	{
		// PlayerState의 유효성 검사
		APlayerState* ClientState = *It;
		if (IsValid(ClientState) == false)
		{
			continue;
		}

		// Session Room 참여 상태를 관리하는 Component 얻기
		URESessionPlayerStateComponent* SessionComponent = ClientState->FindComponentByClass<URESessionPlayerStateComponent>();
		if (IsValid(SessionComponent) == false)
		{
			continue;
		}

		// 클라이언트가 현재 SessionRoom에 참여한 상태인지 확인
		if (SessionComponent->bIsInSessionRoom == false)
		{
			continue;
		}
		
		// SessionRoom에 참여중인 클라이언트의 Ready 상태 확인
		if (SessionComponent->bIsPlayerReady == false)
		{
			return false;
		}
	}
	return true;
}

void URESessionPlayerStateComponent::ServerRequestChangeReadyState_Implementation(bool bNewReadyState)
{
	// 서버에서만 실행되도록 확인
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasAuthority() == false)
	{
		return;
	}
	
	// 변경하려는 값이 동일하면 함수 조기 종료
	if (bIsPlayerReady == bNewReadyState)
	{
		return;
	}

	// 플레이어의 Ready 상태 변경
	bIsPlayerReady = bNewReadyState;

	// Listen 서버 환경 고려 bIsPlayerReady 값 변경 이벤트 실행
	OnRep_IsPlayerReady();
}
