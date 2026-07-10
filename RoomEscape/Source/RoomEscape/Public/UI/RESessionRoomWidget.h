// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RESessionRoomWidget.generated.h"

class UVerticalBox;
class URETextButtonBase;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionButtonClickedSignature);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URESessionRoomWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	// ReadyButton을 
	UPROPERTY(BlueprintAssignable)
	FOnSessionButtonClickedSignature OnButtonClicked_Ready;

protected:
#pragma region Session Room Info

	// Session 이름을 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UTextBlock> TextBlock_SessionName;

	// Session 비밀번호를 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UTextBlock> TextBlock_SessionPassword;

#pragma endregion

#pragma region Player Info

	// Room A에서 스폰될 플레이어가 표시되는 VerticalBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UVerticalBox> VerticalBox_RoomA;

	// Room B에서 스폰될 플레이어가 표시되는 VerticalBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UVerticalBox> VerticalBox_RoomB;

	// 플레이어의 스폰 위치를 Room A로 변경하기 위하여 클릭해야 하는 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<URETextButtonBase> Button_RoomA;

	// 플레이어의 스폰 위치를 Room A로 변경하기 위하여 클릭해야 하는 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<URETextButtonBase> Button_RoomB;

#pragma endregion

#pragma region Session Buttons

	// Session에서 나가는(MainMenu로 복귀) Button
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Back;

	// EOS와 연계되어 다른 사람을 초대하는 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Invite;

	// Ready 또는 Start를 담당하는 Button
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Ready;

#pragma endregion

	// PlayerState를 통하여 Ready상태를 나타내는 TextBlock의 인스턴스를 보관하는 Map
	TMap<int32, UTextBlock*> Map_PlayerTextBlock;

public:
	// UI에 표시되는 Session Room의 이름과 비밀번호를 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetSessionNameAndPassword(FText SessionName, FText SessionPassword);

	// Session Room에 참여한 특정 플레이어를 UI에 추가하는 함수
	UFUNCTION(BlueprintCallable)
	void AddJoinedPlayer(APlayerState* JoinedPlayerState);

	// Session Room에 퇴장한 특정 플레이어를 UI에서 제거하는 함수
	UFUNCTION(BlueprintCallable)
	void RemoveLeavePlayer(APlayerState* LeavePlayerState);

protected:
	// Ready 또는 Start 버튼을 클릭하였을 때 실행되는 함수
	UFUNCTION(BlueprintCallable)
	void OnReadyButtonClicked() { OnButtonClicked_Ready.Broadcast(); }

	// Exit 버튼을 클릭하였을 때 실행되는 함수
	UFUNCTION(BlueprintCallable)
	void OnExitButtonClicked();

	UFUNCTION(BlueprintCallable)
	void ChangePlayerSpawnRoomType(enum ERESpawnRoomType TargetRoomType);

	UFUNCTION()
	void OnPlayerStateChanged(APlayerState* InstigatorState);
};
