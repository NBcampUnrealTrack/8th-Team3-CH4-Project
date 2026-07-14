// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetInitializableInterface.h"
#include "RESessionPlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyStateChangedSignature, APlayerState*, InstigatorState, bool, bNewIsPlayerReady);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOMESCAPE_API URESessionPlayerStateComponent : public UActorComponent, public IWidgetInitializableInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URESessionPlayerStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// IWidgetInitializableInterface을(를) 통해 상속됨
	void InitWidget_Implementation() override;

public:
	// 플레이어의 Ready 상태가 변경되었을 때 실행되는 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnPlayerReadyStateChangedSignature OnReadyStateChanged;

protected:
	// SessionRoom을 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UCommonActivatableWidget> SessionRoomWidgetClass;

	// 현재 플레이어가 SessionRoom에 접속한 상태 여부를 대표하는 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	uint8 bIsInSessionRoom : 1 = false;

	// 플레이어의 Ready상태를 대표하는 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsPlayerReady)
	uint8 bIsPlayerReady : 1 = false;

protected:
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> OwnerPlayerState;

	UPROPERTY(Transient)
	TObjectPtr<class URESessionRoomWidget> SessionRoomWidgetInstance;

public:
	// 플레이어의 Ready 상태를 반환하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsPlayerReady() const { return bIsPlayerReady == true; }

	// SessionRoom에 참여하는 함수
	UFUNCTION(BlueprintCallable)
	void JoinSessionRoom();

	// SessionRoom에서 퇴장하는 함수
	UFUNCTION(BlueprintCallable)
	void LeaveSessionRoom();

protected:
	// (Client 호출) Server에 현재 플레이어(자신)의 Session Room에대한 상태를 변경함을 알리는 함수
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerBroadcastPlayerInSessionRoom(bool bIsJoin);

	// (Server 호출) 연결된 모든 Client들에게 특정 플레이어의 Session Room에대한 상태가 변경되었음을 알리는 함수
	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ClientBroadcastPlayerInSessionRoom(APlayerState* PlayerState, bool bIsJoin);

	UFUNCTION()
	void UpdateAllPlayersInSessionRoom();

	UFUNCTION()
	void OnReadyButtonClicked();

	// (Client 호출) 서버에게 플레이어의 Ready 상태를 변경할 것을 요청하는 함수
	UFUNCTION(Server, Reliable)
	void ServerRequestChangeReadyState(bool bNewReadyState);

	UFUNCTION()
	bool CheckAllPlayerIsReady();

	// bIsPlayerReady 값이 변경되었을 때 호출되는 함수
	UFUNCTION()
	void OnRep_IsPlayerReady() const { OnReadyStateChanged.Broadcast(OwnerPlayerState, bIsPlayerReady); };
};
