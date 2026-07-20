// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SessionRoom/RESessionRoomPlayerState.h"
#include "UI/REWidgetManager.h"
#include "UI/RESessionRoomWidget.h"

//ARESessionRoomPlayerState::ARESessionRoomPlayerState()
//{
//	SetReplicates(true);
//}
//
//void ARESessionRoomPlayerState::BeginPlay()
//{
//	Super::BeginPlay();
//
//}
//
//void ARESessionRoomPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}
//
//void ARESessionRoomPlayerState::JoinSessionRoom()
//{
//	// 기존에 Session Room에 참여중인 플레이어 UI 등록
//	UpdateAllPlayersInSessionRoom();
//
//	// 서버에 플레이어가 Session Room에 참여하였음을 알림
//	ServerBroadcastPlayerInSessionRoom(true);
//}
//
//void ARESessionRoomPlayerState::InitWiget()
//{
//	// 로컬 플레이어의 유효성 확인
//	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetNetOwningPlayer());
//	if (IsValid(LocalPlayer) == false)
//	{
//		return;
//	}
//
//	// UI 매니저 유효성 확인
//	UREWidgetManager* WidgetManager = LocalPlayer->GetSubsystem<UREWidgetManager>();
//	if (IsValid(WidgetManager) == false)
//	{
//		return;
//	}
//
//	// UI 생성 및 유효성 확인
//	UCommonActivatableWidget* WidgetInstance = WidgetManager->AddWiget(SessionRoomWidgetData);
//	SessionRoomWidgetInstance = Cast<URESessionRoomWidget>(WidgetInstance);
//	if (IsValid(SessionRoomWidgetInstance) == false)
//	{
//		return;
//	}
//
//	// 버튼 클릭 이벤트 함수 연결
//	SessionRoomWidgetInstance->OnButtonClicked_Ready.AddUniqueDynamic(this, &ThisClass::OnReadyButtonClicked);
//}
