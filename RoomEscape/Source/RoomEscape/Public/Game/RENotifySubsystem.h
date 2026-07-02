// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "RENotifySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnREGameEventSignature, FGameplayTag, EventTag, const FString&, Message);

/**
 * 게임 내 주요 이벤트 알림의 단일 창구.
 * 현재는 LogREEvent 카테고리로 콘솔 로그만 출력 (디버그 전용).
 * 추후 UI 알림이 필요해지면 OnGameEvent 델리게이트를 구독하는 방식으로 확장.
 */
UCLASS(BlueprintType)
class ROOMESCAPE_API URENotifySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Notify")
	FOnREGameEventSignature OnGameEvent;

public:
	UFUNCTION(BlueprintPure, Category = "Notify", meta = (WorldContext = "WorldContextObject"))
	static URENotifySubsystem* GetInstance(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Notify")
	void NotifyEvent(FGameplayTag EventTag, const FString& Message);

private:
	// 로그가 서버/클라 어느 쪽에서 찍혔는지 구분하기 위한 접두사
	FString GetNetModePrefix() const;
};
