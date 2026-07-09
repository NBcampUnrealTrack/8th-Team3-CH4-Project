// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/InitializeUtilityInterface.h"
#include "RESessionRoomWidget.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionButtonClickedSignature);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URESessionRoomWidget : public UCommonActivatableWidget, public IInitializeUtilityInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// IInitializeUtilityInterface을(를) 통해 상속됨
	void InitializeWidgetByContextObject_Implementation(UObject* ContextObject) override;

	void InitializeWidgetByComponent_Implementation(UActorComponent* Component) override;

	void InitializeWidgetByActor_Implementation(AActor* Actor) override;

public:
	// ReadyButton을 
	UPROPERTY(BlueprintAssignable)
	FOnSessionButtonClickedSignature OnButtonClicked_Ready;

protected:
	// Session 이름을 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UTextBlock> TextBlock_SessionName;

	// Session 비밀번호를 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UTextBlock> TextBlock_SessionPassword;

	// 현재 Session에 접속한 Player 목록을 보관하는 ScrollBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<class UScrollBox> ScrollBox_Players;

	// Session에서 나가는(MainMenu로 복귀) Button
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Back;

	// EOS와 연계되어 다른 사람을 초대하는 버튼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Invite;

	// Ready 또는 Start를 담당하는 Button
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UButton> Button_Ready;

	// PlayerState를 통하여 Ready상태를 나타내는 TextBlock의 인스턴스를 보관하는 Map
	TMap<int32, UTextBlock*> Map_PlayerTextBlock;

public:
	UFUNCTION(BlueprintCallable)
	void AddJoinedPlayer(APlayerState* JoinedPlayerState);

	UFUNCTION(BlueprintCallable)
	void RemoveLeavePlayer(APlayerState* LeavePlayerState);

protected:
	UFUNCTION(BlueprintCallable)
	void OnReadyButtonClicked() { OnButtonClicked_Ready.Broadcast(); }

	UFUNCTION()
	void OnReadyStateChanged(APlayerState* InstigatorState, bool bNewIsPlayerReady);

};
