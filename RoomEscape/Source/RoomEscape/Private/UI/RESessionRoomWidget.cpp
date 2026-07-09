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

void URESessionRoomWidget::InitializeWidgetByContextObject_Implementation(UObject* ContextObject)
{
}

void URESessionRoomWidget::InitializeWidgetByComponent_Implementation(UActorComponent* Component)
{
	URESessionPlayerStateComponent* SessionRoomComponent = Cast<URESessionPlayerStateComponent>(Component);
	if (IsValid(SessionRoomComponent) == false)
	{
		return;
	}

	SessionRoomComponent->OnReadyStateChanged.AddUniqueDynamic(this, &ThisClass::OnReadyStateChanged);
}

void URESessionRoomWidget::InitializeWidgetByActor_Implementation(AActor* Actor)
{
}

void URESessionRoomWidget::OnPostJoinPlayer(const APlayerState* JoinedPlayerState)
{
	// Scroll Box 또는 PlayerState가 유효하지 않으면 함수 조기 종료
	if (IsValid(ScrollBox_Players) == false || IsValid(JoinedPlayerState) == false)
	{
		return;
	}

	// 플레이어의 Ready 상태를 담당하는 컴포넌트 참조 및 유효성 확인
	URESessionPlayerStateComponent* SessionComponent = JoinedPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionComponent) == false)
	{
		return;
	}

	// 플레이어 고유의 ID 얻기
	int32 PlayerId = JoinedPlayerState->GetPlayerId();

	// 해당 플레이어를 담당하는 TextBlock이 존재하면 Ready 상태 변경에 대한 함수 실행
	if (Map_PlayerTextBlock.Contains(PlayerId) == true)
	{
		OnReadyStateChanged(SessionComponent, SessionComponent->IsPlayerReady());
		return;
	}

	// 플레이어 이름, 준비 상태를 표시할 TextBlock 생성 및 유효성 검사
	UTextBlock* TextBlock_Player = WidgetTree->ConstructWidget<UTextBlock>();
	if (IsValid(TextBlock_Player) == false)
	{
		return;
	}

	// 표시될 Text 설정
	FString DisplayString = JoinedPlayerState->GetPlayerName();
	if (SessionComponent->IsPlayerReady() == true)
	{
		DisplayString.Append(TEXT("  (Ready)"));
	}
	TextBlock_Player->SetText(FText::FromString(DisplayString));

	// ScrollBox에 플레이어 표시
	ScrollBox_Players->AddChild(TextBlock_Player);

	// 플레이어를 담당하는 TextBlock의 Instance 저장
	Map_PlayerTextBlock.Add(PlayerId, TextBlock_Player);
	return;
}

void URESessionRoomWidget::OnPreLeavePlayer(const APlayerState* LeavePlayerState)
{
	// 플레이어 고유의 ID 얻기
	int32 PlayerId = LeavePlayerState->GetPlayerId();
	UTextBlock* TextBlock_Player = nullptr;

	// 해당 플레이어를 담당하는 TextBlock 얻기
	if (Map_PlayerTextBlock.RemoveAndCopyValue(PlayerId, TextBlock_Player) == false)
	{
		return;
	}

	// 플레이어가 Session을 나갔음을 표시(Player 담당 TextBlock 제거)
	TextBlock_Player->RemoveFromParent();
	return;
}

void URESessionRoomWidget::OnReadyStateChanged(const UActorComponent* InstigatorComponent, bool bNewIsPlayerReady)
{
	if (IsValid(InstigatorComponent) == false)
	{
		return;
	}

	APlayerState* InstigatorState = Cast<APlayerState>(InstigatorComponent->GetOwner());
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
