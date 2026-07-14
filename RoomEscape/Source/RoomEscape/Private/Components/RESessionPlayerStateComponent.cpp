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
	DOREPLIFETIME(ThisClass, SpawnRoomType);
}

void URESessionPlayerStateComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	// Session Room Widget 비활성화
	if (IsValid(SessionRoomWidgetInstance) == true)
	{
		SessionRoomWidgetInstance->DeactivateWidget();
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
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

	// 기존에 Session Room에 참여중인 플레이어 UI 등록
	UpdateAllPlayersInSessionRoom();

	// 서버에 플레이어가 Session Room에 참여하였음을 알림
	ServerBroadcastPlayerInSessionRoom(true);
}

void URESessionPlayerStateComponent::LeaveSessionRoom()
{
	// 서버에 플레이어가 Session Room에 퇴장하였음을 알림
	ServerBroadcastPlayerInSessionRoom(false);

	// Session Room UI 제거
	if (IsValid(SessionRoomWidgetInstance) == true)
	{
		SessionRoomWidgetInstance->DeactivateWidget();
	}
}

void URESessionPlayerStateComponent::ChangeSpawnRoomType(ERESpawnRoomType TargetRoomType)
{
	// Local Player 환경 실행 검사
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasLocalNetOwner() == false)
	{
		return;
	}

	// 서버에게 플레이어가 스폰될 방을 변경한다고 알림
	ServerRequestChangeSpawnRoomType(TargetRoomType);
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

	// Session Room 참여 또는 퇴장 시 Ready 상태는 false로 고정 변경
	bIsPlayerReady = false;

	// Session Room 참여 시 Spawn Room Type은 Alpha로 설정, 퇴장 시 None으로 설정
	SpawnRoomType = bIsJoin == true ? ERESpawnRoomType::Alpha : ERESpawnRoomType::None;

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
			SessionComponent->ClientBroadcastPlayerInSessionRoom(OwnerPlayerState, bIsJoin);
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

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, *FString::Printf(TEXT("New Client joined in Session - %s"), *PlayerState->GetPlayerName()));

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

void URESessionPlayerStateComponent::UpdateAllPlayersInSessionRoom()
{
	if (IsValid(SessionRoomWidgetInstance) == false)
	{
		return;
	}

	// World에 존재하는 모든 PlayerState 중에서 Session에 참여되어있는 모든 Client 검색
	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
	{
		// PlayerState의 유효성 검사
		APlayerState* ClientState = *It;
		if (IsValid(ClientState) == false)
		{
			continue;
		}

		// Session Room 참여 상태를 관리하는 Component 얻기
		URESessionPlayerStateComponent* SessionRoomComponent = ClientState->FindComponentByClass<URESessionPlayerStateComponent>();
		if (IsValid(SessionRoomComponent) == false)
		{
			continue;
		}

		// 클라이언트가 현재 SessionRoom에 참여한 상태인지 확인
		if (SessionRoomComponent->bIsInSessionRoom == true)
		{
			// 해당 플레이어가 현재 SessionRoom UI에 추가
			SessionRoomWidgetInstance->AddJoinedPlayer(ClientState);
		}
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
	if (OwnerPlayerState->HasAuthority() == true)
	{
		// Session Room에 참여한 전체 플레이어의 Ready 상태 검사
		bool bAllPlayerIsReady = CheckAllPlayerIsReady();

		// 참여한 전체 플레이어가 Ready 상태일 경우 게임 맵 로드
		if (bAllPlayerIsReady == true)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("# URESessionPlayerStateComponent - Need to open Game Map"));
			StartOpenGameMap(GameMap);
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

bool URESessionPlayerStateComponent::CheckAllPlayerIsReady() const
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
		if (IsValid(SessionComponent) == false || SessionComponent == this)
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

void URESessionPlayerStateComponent::OnRep_IsPlayerReady() const { OnSessionPlayerStateChanged.Broadcast(OwnerPlayerState); }

void URESessionPlayerStateComponent::OnRep_SpawnRoomType() const { OnSessionPlayerStateChanged.Broadcast(OwnerPlayerState); }

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

void URESessionPlayerStateComponent::ServerRequestChangeSpawnRoomType_Implementation(ERESpawnRoomType TargetRoomType)
{
	// 서버 환경 실행 검사
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasAuthority() == false)
	{
		return;
	}

	// 플레이어가 Ready 상태 또는 현재의 자신과 동일한 SpawnRoomType으로 변경하려는 RoomType 변경 금지
	if (bIsPlayerReady == true || SpawnRoomType == TargetRoomType)
	{
		return;
	}

	// RoomType 값 변경
	SpawnRoomType = TargetRoomType;

	// 변경된 RoomType이 None이면 함수 조기 종료
	if (SpawnRoomType == ERESpawnRoomType::None)
	{
		return;
	}

	// Listen 서버 환경 고려 SpawnRoomType 값 변경 이벤트 실행
	OnRep_SpawnRoomType();
}

bool URESessionPlayerStateComponent::StartOpenGameMap(TSoftObjectPtr<UWorld> Map)
{
	// 서버 권한을 가지고 있는지 확인합니다.
	if (IsValid(OwnerPlayerState) == false || OwnerPlayerState->HasAuthority() == false)
	{
		return false;
	}

	if (Map.IsNull() == true)
	{
		return false;
	}

	// 이미 맵 이동을 요청했는지 확인합니다.
	if (bIsTravelRequested == true)
	{
		return false;
	}

	// 로드할 맵의 Soft Object Path를 문자열로 가져옵니다.
	FString GameMapObjectPath = Map.ToSoftObjectPath().ToString();

	// 맵 경로가 비어 있는지 확인합니다.
	if (GameMapObjectPath.IsEmpty() == true)
	{
		return false;
	}

	// 오브젝트 경로를 패키지 경로로 변환합니다.
	FString GameMapPackageName = FPackageName::ObjectPathToPackageName(GameMapObjectPath);

	// 패키지 경로가 유효한지 확인합니다.
	if (FPackageName::IsValidLongPackageName(GameMapPackageName) == false)
	{
		return false;
	}

	// 현재 World를 가져옵니다.
	UWorld* World = GetWorld();

	// World가 유효하지 않은지 확인합니다.
	if (IsValid(World) == false)
	{
		return false;
	}

	// 중복 이동 요청 방지 플래그를 설정합니다.
	bIsTravelRequested = true;

	// Client에서 Map Load를 위하여 문자열 추가
	GameMapPackageName = GameMapPackageName.Append(TEXT("?listen"));

	// 선택한 맵으로 ServerTravel을 실행합니다.
	bool bTravelStarted = World->ServerTravel(GameMapPackageName, false);

	// ServerTravel 실행에 실패했는지 확인합니다.
	if (bTravelStarted == false)
	{
		// 다시 시도할 수 있도록 플래그를 초기화합니다.
		bIsTravelRequested = false;

		UE_LOG(LogTemp, Error, TEXT("URESessionPlayerStateComponent: ServerTravel 실패 - %s"), *GameMapPackageName);
	}

	return bTravelStarted;
}
