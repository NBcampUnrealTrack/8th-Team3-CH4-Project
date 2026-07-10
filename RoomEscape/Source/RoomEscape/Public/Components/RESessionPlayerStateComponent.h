// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetInitializableInterface.h"
#include "RESessionPlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionPlayerStateChangedSignature, APlayerState*, InstigatorState);

UENUM(BlueprintType)
enum class ERESpawnRoomType : uint8
{
	None,
	Alpha,
	Beta
};

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

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:	
	// IWidgetInitializableInterface을(를) 통해 상속됨
	void InitWidget_Implementation() override;

public:
	// SessionRoom에 참여한 플레이어의 상태가 변경되었을 때 실행되는 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnSessionPlayerStateChangedSignature OnSessionPlayerStateChanged;

protected:
	// 컴포넌트를 소유한 PlayerState
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> OwnerPlayerState;

	// 생성된 Session Room UI의 Instance
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Session Room")
	TSubclassOf<class UCommonActivatableWidget> SessionRoomWidgetClass;

	// 현재 플레이어가 SessionRoom에 접속한 상태 여부를 대표하는 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Session Room")
	uint8 bIsInSessionRoom : 1 = false;

	// 플레이어의 Ready상태를 대표하는 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsPlayerReady, Category = "Session Room")
	uint8 bIsPlayerReady : 1 = false;

	// 플레이어가 스폰될 장소
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_SpawnRoomType, Category = "Session Room")
	ERESpawnRoomType SpawnRoomType = ERESpawnRoomType::None;

	// 게임 시작 시 로드되는 맵
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Session Room")
	TSoftObjectPtr<UWorld> GameMap;

	// 생성된 Session Room UI의 Instance
	UPROPERTY(Transient)
	TObjectPtr<class URESessionRoomWidget> SessionRoomWidgetInstance;

	// 중복 ServerTravel 호출을 방지하는 플래그
	UPROPERTY(Transient)
	uint8 bIsTravelRequested : 1 = false;

public:
	// 플레이어가 Session Room에 참여중인지 여부를 반환하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsPlayerInSession() const { return bIsInSessionRoom == true; }

	// 플레이어의 Ready 상태를 반환하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsPlayerReady() const { return bIsPlayerReady == true; }

	// 플레이어의 SpawnRoomType을 반환하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE ERESpawnRoomType GetPlayerSpawnRoomType() const { return SpawnRoomType; }

	// SessionRoom에 참여하는 함수
	UFUNCTION(BlueprintCallable)
	void JoinSessionRoom();

	// SessionRoom에서 퇴장하는 함수
	UFUNCTION(BlueprintCallable)
	void LeaveSessionRoom();

	// 게임 시작 시 플레이어가 스폰될 장소를 변경하는 함수
	UFUNCTION(BlueprintCallable)
	void ChangeSpawnRoomType(ERESpawnRoomType TargetRoomType);

protected:
	// (Client 호출) Server에 현재 플레이어(자신)의 Session Room에대한 상태를 변경함을 알리는 함수
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerBroadcastPlayerInSessionRoom(bool bIsJoin);

	// (Server 호출) 연결된 모든 Client들에게 특정 플레이어의 Session Room에대한 상태가 변경되었음을 알리는 함수
	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ClientBroadcastPlayerInSessionRoom(APlayerState* PlayerState, bool bIsJoin);

	// Session Room에 참여한 전체 플레이어 정보를 UI에 추가 또는 갱신하는 함수
	UFUNCTION()
	void UpdateAllPlayersInSessionRoom();

	// UI의 Ready Button 클릭 시 실행되는 함수
	UFUNCTION()
	void OnReadyButtonClicked();

	// Session Room에 참여한 전체 플레이어가 현재 Ready 상태인지 확인하는 함수
	UFUNCTION()
	bool CheckAllPlayerIsReady() const;

	// bIsPlayerReady 값이 변경되었을 때 호출되는 함수
	UFUNCTION()
	void OnRep_IsPlayerReady() const;

	// SpawnRoomType 값이 변경되었을 때 호출되는 함수
	UFUNCTION()
	void OnRep_SpawnRoomType() const;

	// (Client 호출) 서버에게 플레이어의 Ready 상태를 변경할 것을 요청하는 함수
	UFUNCTION(Server, Reliable)
	void ServerRequestChangeReadyState(bool bNewReadyState);

	// 서버가 플레이어를 지정된 방으로 이동시킵니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestChangeSpawnRoomType(ERESpawnRoomType TargetRoomType);

	// 임의 맵으로 ServerTravel을 실행하는 함수
	UFUNCTION(BlueprintCallable)
	bool StartOpenGameMap(TSoftObjectPtr<UWorld> Map);
};
