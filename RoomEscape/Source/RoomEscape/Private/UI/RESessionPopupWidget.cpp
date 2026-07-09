// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RESessionPopupWidget.h"
#include "Components/EditableTextBox.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UI/RETextButtonBase.h"
#include "Game/REGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Components/RESessionPlayerStateComponent.h"

void URESessionPopupWidget::AddPopupButton_HostGame()
{
	URETextButtonBase* Button_HostGame = AddPopupButton(FText::FromString(TEXT("Create Session")));
	Button_HostGame->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::HostGameSession);
}

void URESessionPopupWidget::HostGameSession()
{
	if (IsValid(InputField_SessionName) == false || IsValid(InputField_SessionPassword) == false)
	{
		return;
	}

	// 사용자가 입력한 Session Name 얻기 및 유효성 검사
	FString SessionName = InputField_SessionName->GetText().ToString();
	SessionName = SessionName.TrimStartAndEnd();
	if (SessionName.IsEmpty() == true)
	{
		return;
	}

	// 사용자가 입력한 Session Password 얻기
	FString SessionPassword = InputField_SessionName->GetText().ToString();
	SessionPassword = SessionPassword.TrimStartAndEnd();

	// 게임 인스턴스 얻기
	UREGameInstance* GameInstance = GetGameInstance<UREGameInstance>();
	if (IsValid(GameInstance) == false)
	{
		DeactivateWidget();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("# Null Reference Error on GameInstance"));
		UE_LOG(LogTemp, Error, TEXT("# Null Reference Error on GameInstance"));
		return;
	}

	// 입력받은 Session 이름과 비밀번호를 사용하여 Game Host 시작
	GameInstance->HostGame(SessionName, SessionPassword);
	DeactivateWidget();

	// SessionRoom 참여 이벤트 실행
	OnCreateOrJoinSessionRoom();
}

void URESessionPopupWidget::AddPopupButton_JoinGame()
{
	URETextButtonBase* Button_JoinGame = AddPopupButton(FText::FromString(TEXT("Join Session")));
	Button_JoinGame->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::JoinGameSession);
}

void URESessionPopupWidget::JoinGameSession()
{
	if (IsValid(InputField_SessionName) == false || IsValid(InputField_SessionPassword) == false)
	{
		return;
	}

	// 사용자가 입력한 Session Name 얻기 및 유효성 검사
	FString SessionName = InputField_SessionName->GetText().ToString();
	SessionName = SessionName.TrimStartAndEnd();
	if (SessionName.IsEmpty() == true)
	{
		return;
	}

	// 사용자가 입력한 Session Password 얻기
	FString SessionPassword = InputField_SessionName->GetText().ToString();
	SessionPassword = SessionPassword.TrimStartAndEnd();

	// 게임 인스턴스 얻기
	UREGameInstance* GameInstance = GetGameInstance<UREGameInstance>();
	if (IsValid(GameInstance) == false)
	{
		DeactivateWidget();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("# Null Reference Error on GameInstance"));
		UE_LOG(LogTemp, Error, TEXT("# Null Reference Error on GameInstance"));
		return;
	}

	// 입력받은 Session 이름과 비밀번호를 사용하여 Game Host 시작
	GameInstance->JoinGame(SessionName, SessionPassword);
	DeactivateWidget();

	// SessionRoom 참여 이벤트 실행
	OnCreateOrJoinSessionRoom();
}

void URESessionPopupWidget::OnCreateOrJoinSessionRoom()
{
	// Local PlayerController 얻기
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (IsValid(LocalPlayerController) == false)
	{
		return;
	}

	// Local Player의 PlayerState 얻기
	APlayerState* LocalPlayerState = LocalPlayerController->PlayerState;
	if (IsValid(LocalPlayerState) == false)
	{
		return;
	}

	// (Local)PlayerState의 Session Room 상태를 관리하는 Component 얻기
	URESessionPlayerStateComponent* SessionComponent = LocalPlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
	if (IsValid(SessionComponent) == false)
	{
		return;
	}

	// SessionRoom에 참여했다고 알림
	SessionComponent->JoinSessionRoom();
}
