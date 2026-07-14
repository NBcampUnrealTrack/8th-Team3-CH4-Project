// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RESessionRoomWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/PlayerState.h"
#include "Components/RESessionPlayerStateComponent.h"

void URESessionRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Button_Ready) == true)
	{
		Button_Ready->OnClicked.AddDynamic(this, &ThisClass::OnReadyButtonClicked);
	}
}

void URESessionRoomWidget::AddJoinedPlayer(APlayerState* JoinedPlayerState)
{
	// Scroll Box 또는 PlayerState가 유효하지 않으면 함수 조기 종료
	if (IsValid(ScrollBox_Players) == false || IsValid(JoinedPlayerState) == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Layout"));
		return;
	}

	// 플레이어의 Ready 상태를 관리하는 Component 확인
	URESessionPlayerStateComponent* SessionRoomComponent = JoinedPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionRoomComponent) == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No SessionRoomComponent"));
		return;
	}

	// 플레이어를 대표하는 ID 얻기(플레이어 인덱스)
	int32 PlayerID = JoinedPlayerState->GetPlayerId();

	// 참여한 플레이어를 대표하는 TextBlock이 존재하면 해당 TextBlock 업데이트
	if (Map_PlayerTextBlock.Contains(PlayerID) == true)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Exsited Player"));
		OnReadyStateChanged(JoinedPlayerState, SessionRoomComponent->IsPlayerReady());
		return;
	}

	// 참여한 플레이어의 Ready 상태를 나타내는 TextBlock 생성
	UTextBlock* TextBlock_Player = WidgetTree->ConstructWidget<UTextBlock>();
	if (IsValid(TextBlock_Player) == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("TextBlock_Player Construct Failed"));
		return;
	}

	// 표시될 Text 설정
	FString DisplayString = JoinedPlayerState->GetPlayerName();
	if (SessionRoomComponent->IsPlayerReady() == true)
	{
		// Ready 상태일 경우 Ready 문구 추가
		DisplayString.Append(TEXT("  (Ready)"));
	}

	// TextBlock의 Text 업데이트
	TextBlock_Player->SetText(FText::FromString(DisplayString));

	// ScrollBox에 플레이어 담당 TextBlock 추가
	ScrollBox_Players->AddChild(TextBlock_Player);

	// 플레이어를 담당하는 TextBlock의 Instance 저장
	Map_PlayerTextBlock.Add(PlayerID, TextBlock_Player);

	// 플레이어의 Ready 상태 변경에 대한 이벤트 연결
	SessionRoomComponent->OnReadyStateChanged.AddUniqueDynamic(this, &ThisClass::OnReadyStateChanged);
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

void URESessionRoomWidget::OnReadyStateChanged(APlayerState* InstigatorState, bool bNewIsPlayerReady)
{
	if (IsValid(InstigatorState) == false)
	{
		return;
	}

	int32 PlayerID = InstigatorState->GetPlayerId();
	if (Map_PlayerTextBlock.Contains(PlayerID) == false)
	{
		return;
	}

	UTextBlock* TextBlock_Player = Map_PlayerTextBlock[PlayerID];
	if (IsValid(TextBlock_Player) == false)
	{
		return;
	}

	FString DisplayString = InstigatorState->GetPlayerName();
	if (bNewIsPlayerReady == true)
	{
		DisplayString.Append(TEXT("  (Ready)"));
	}
	TextBlock_Player->SetText(FText::FromString(DisplayString));
}
