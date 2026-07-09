// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetInitializableInterface.h"
#include "RESessionPlayerStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyStateChangedSignature, const UActorComponent*, InstigatorComponent, bool, bNewIsPlayerReady);

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

	// 플레이어의 Ready상태를 대표하는 플래그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_IsPlayerReady)
	uint8 bIsPlayerReady : 1 = false;

private:
	TObjectPtr<APlayerState> OwnerPlayerState;

public:
	// 플레이어의 Ready 상태를 반환하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsPlayerReady() const { return bIsPlayerReady == true; }

	bool CheckAllPlayerIsReady();

	// (Client 호출)서버에 플레이어의 Ready 상태를 변경할 것을 요청하는 함수
	// (Server 호출)플레이어의 Ready 상태를 변경하는 함수
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerRequestChangeReadyState();

protected:
	// bIsPlayerReady 플래그 값이 Net상에서 변경되면 호출되는 함수
	UFUNCTION(BlueprintCallable)
	void OnRep_IsPlayerReady() const { OnReadyStateChanged.Broadcast(this, bIsPlayerReady); }

	// 플레이어가 SessionRoom UI의 Ready 버튼을 클릭하였을 때 호출하는 함수
	UFUNCTION()
	void OnReadyButtonClicked();
};
