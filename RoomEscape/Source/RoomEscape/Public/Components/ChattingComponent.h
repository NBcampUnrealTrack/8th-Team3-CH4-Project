// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetInitializableInterface.h"
#include "ChattingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMessageReceivedSignature, const struct FGameplayTag&, ChannelTag, const FString&, Sender, const FString&, Message);

/*
* Chatting을 위하여 PlayerController에 추가하는 컴포넌트
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOMESCAPE_API UChattingComponent : public UActorComponent, public IWidgetInitializableInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UChattingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// IWidgetInitializableInterface을(를) 통해 상속됨
	void InitWidget_Implementation() override;

public:
	// 서버로부터 메시지를 수신하였을 경우 실행되는 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnMessageReceivedSignature OnMessageReceived;

protected:
	// 컴포넌트를 소유하게 될 PlayerController
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	APlayerController* OwnerController;

	// 채팅 UI 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UChatBoxWidget> ChattingWidgetClass;

protected:
	// (Client 호출, Server 실행) 메시지를 전송하는 함수
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerOnMessageCommitted(const struct FGameplayTag& ChannelTag, const FString& Message);

	// (Server 호출, Client 실행) 메시지를 전송하는 함수
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientOnMessageReceived(const struct FGameplayTag& ChannelTag, const FString& Sender, const FString& Message);
};
