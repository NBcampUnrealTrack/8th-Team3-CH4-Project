// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RESessionRoomWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/RETextButtonBase.h"
#include "GameFramework/PlayerState.h"
#include "Components/RESessionPlayerStateComponent.h"

void URESessionRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Button_Ready) == true)
	{
		Button_Ready->OnClicked().RemoveAll(this);
		Button_Ready->OnClicked().AddUObject(this, &ThisClass::OnReadyButtonClicked);
	}
	if (IsValid(Button_Exit) == true)
	{
		Button_Exit->OnClicked().RemoveAll(this);
		Button_Exit->OnClicked().AddUObject(this, &ThisClass::OnExitButtonClicked);
	}
}

void URESessionRoomWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void URESessionRoomWidget::NativeOnDeactivated()
{
	RemoveLeavePlayer(GetOwningPlayerState());
	Super::NativeOnDeactivated();
}

void URESessionRoomWidget::SetSessionNameAndPassword(FText SessionName, FText SessionPassword)
{
	if (IsValid(TextBlock_SessionName) == true)
	{
		TextBlock_SessionName->SetText(SessionName);
	}
	if (IsValid(TextBlock_SessionPassword) == true)
	{
		TextBlock_SessionPassword->SetText(SessionPassword);
	}
}

void URESessionRoomWidget::AddJoinedPlayer(APlayerState* JoinedPlayerState)
{
	// Vertical Box 또는 PlayerState가 유효하지 않으면 함수 조기 종료
	if (IsValid(VerticalBox_RoomA) == false || IsValid(VerticalBox_RoomB) == false || IsValid(JoinedPlayerState) == false)
	{
		return;
	}

	// 플레이어의 Ready 상태를 관리하는 Component 확인
	URESessionPlayerStateComponent* SessionRoomComponent = JoinedPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionRoomComponent) == false)
	{
		return;
	}

	// 플레이어를 대표하는 ID 얻기(플레이어 인덱스)
	int32 PlayerID = JoinedPlayerState->GetPlayerId();

	// 참여한 플레이어를 대표하는 TextBlock이 존재하면 해당 TextBlock 업데이트
	if (Map_PlayerTextBlock.Contains(PlayerID) == true)
	{
		OnPlayerStateChanged(JoinedPlayerState);
		return;
	}

	// 참여한 플레이어의 Ready 상태를 나타내는 TextBlock 생성
	UTextBlock* TextBlock_Player = WidgetTree->ConstructWidget<UTextBlock>();
	if (IsValid(TextBlock_Player) == false)
	{
		return;
	}

	// 텍스트 래핑 설정
	TextBlock_Player->SetAutoWrapText(true);

	// 표시될 Text 설정
	FString DisplayString = JoinedPlayerState->GetPlayerName();
	if (SessionRoomComponent->IsPlayerReady() == true)
	{
		// Ready 상태일 경우 Ready 문구 추가
		DisplayString.Append(TEXT("  (Ready)"));
	}

	// TextBlock의 Text 업데이트
	TextBlock_Player->SetText(FText::FromString(DisplayString));

	// 플레이어의 SpawnRoopType에 따라 UI 위치 설정
	if (SessionRoomComponent->GetPlayerSpawnRoomType() == ERESpawnRoomType::Alpha)
	{
		if (VerticalBox_RoomA->HasChild(TextBlock_Player) == false)
		{
			VerticalBox_RoomA->AddChildToVerticalBox(TextBlock_Player);
		}
	}
	if (SessionRoomComponent->GetPlayerSpawnRoomType() == ERESpawnRoomType::Beta)
	{
		if (VerticalBox_RoomB->HasChild(TextBlock_Player) == false)
		{
			VerticalBox_RoomB->AddChildToVerticalBox(TextBlock_Player);
		}
	}

	// 플레이어를 담당하는 TextBlock의 Instance 저장
	Map_PlayerTextBlock.Add(PlayerID, TextBlock_Player);

	// 플레이어의 상태 변경에 대한 이벤트 연결
	SessionRoomComponent->OnSessionPlayerStateChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerStateChanged);
}

void URESessionRoomWidget::RemoveLeavePlayer(APlayerState* LeavePlayerState)
{
	// 플레이어 고유의 ID 얻기
	int32 PlayerID = LeavePlayerState->GetPlayerId();
	UTextBlock* TextBlock_Player = nullptr;

	// 해당 플레이어를 담당하는 TextBlock 얻기
	// 플레이어를 담당하는 TextBlock을 Map에서 제거
	if (Map_PlayerTextBlock.RemoveAndCopyValue(PlayerID, TextBlock_Player) == false)
	{
		return;
	}

	// 플레이어가 Session을 나갔음을 표시(Player 담당 TextBlock 제거)
	TextBlock_Player->RemoveFromParent();
	return;
}

void URESessionRoomWidget::OnExitButtonClicked()
{
	// Local Player의 PlayerState 얻기
	APlayerState* LocalPlayerState = GetOwningPlayerState();
	if (IsValid(LocalPlayerState) == false)
	{
		return;
	}

	// Local PlayerState의 SessionRoom 관리 Component 얻기
	URESessionPlayerStateComponent* SessionRoomComponent = LocalPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionRoomComponent) == false)
	{
		return;
	}

	// SessionRoom에서 퇴장함을 알림
	SessionRoomComponent->LeaveSessionRoom();

	// SessionRoom Widget 비활성화
	DeactivateWidget();
}

void URESessionRoomWidget::ChangePlayerSpawnRoomType(ERESpawnRoomType TargetRoomType)
{
	APlayerState* LocalPlayerState = GetOwningPlayerState();
	if (IsValid(LocalPlayerState) == false)
	{
		return;
	}

	// SessionRoom 관리 Component 얻기
	URESessionPlayerStateComponent* SessionRoomComponent = LocalPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionRoomComponent) == false)
	{
		return;
	}

	// Spawn Room Type 변경 함수 실행
	SessionRoomComponent->ChangeSpawnRoomType(TargetRoomType);
}

void URESessionRoomWidget::OnPlayerStateChanged(APlayerState* InstigatorState)
{
	// 이벤트를 실행시킨 PlayerState의 유효성 검사
	if (IsValid(InstigatorState) == false)
	{
		return;
	}

	// SessionRoom 관리 Component 얻기
	URESessionPlayerStateComponent* SessionRoomComponent = InstigatorState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionRoomComponent) == false)
	{
		return;
	}

	// 플레이어가 Session Room에 참여한 상태인지 확인
	// 플레이어의 Spawn Room Type이 유효한지 확인
	if (SessionRoomComponent->IsPlayerInSession() == false || SessionRoomComponent->GetPlayerSpawnRoomType() == ERESpawnRoomType::None)
	{
		return;
	}

	// 플레이어를 대표하는 ID(Index) 번호 얻기
	int32 PlayerID = InstigatorState->GetPlayerId();

	// 해당 플레이어에 대한 상태를 나타내는 TextBlock 찾기
	UTextBlock* TextBlock_Player = Map_PlayerTextBlock.FindRef(PlayerID, nullptr);

	// 검색된 TextBlock이 유효하지 않으면 새로 생성
	if (IsValid(TextBlock_Player) == false)
	{
		TextBlock_Player = WidgetTree->ConstructWidget<UTextBlock>();

		// 텍스트 래핑 설정
		TextBlock_Player->SetAutoWrapText(true);
	}

	// UI에 표시될 Text 설정
	FString DisplayString = InstigatorState->GetPlayerName();
	if (SessionRoomComponent->IsPlayerReady() == true)
	{
		DisplayString.Append(TEXT("  (Ready)"));
	}
	TextBlock_Player->SetText(FText::FromString(DisplayString));

	// 플레이어의 SpawnRoopType에 따라 UI 위치 설정
	if (SessionRoomComponent->GetPlayerSpawnRoomType() == ERESpawnRoomType::Alpha)
	{
		if (VerticalBox_RoomA->HasChild(TextBlock_Player) == false)
		{
			VerticalBox_RoomA->AddChildToVerticalBox(TextBlock_Player);
		}
		return;
	}
	if (SessionRoomComponent->GetPlayerSpawnRoomType() == ERESpawnRoomType::Beta)
	{
		if (VerticalBox_RoomB->HasChild(TextBlock_Player) == false)
		{
			VerticalBox_RoomB->AddChildToVerticalBox(TextBlock_Player);
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("URESessionPlayerStateComponent::SpawnRoomType is Invalid Value!"));
}
