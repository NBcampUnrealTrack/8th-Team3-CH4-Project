// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/RENotifySubsystem.h"
#include "RoomEscape.h"
#include "Engine/World.h"

URENotifySubsystem* URENotifySubsystem::GetInstance(const UObject* WorldContextObject)
{
	if (IsValid(WorldContextObject) == false)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	return IsValid(GameInstance) == true ? GameInstance->GetSubsystem<URENotifySubsystem>() : nullptr;
}

void URENotifySubsystem::NotifyEvent(FGameplayTag EventTag, const FString& Message)
{
	UE_LOG(LogREEvent, Log, TEXT("[%s] %s : %s"), *GetNetModePrefix(), *EventTag.ToString(), *Message);

	OnGameEvent.Broadcast(EventTag, Message);
}

FString URENotifySubsystem::GetNetModePrefix() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UWorld* World = IsValid(GameInstance) == true ? GameInstance->GetWorld() : nullptr;
	if (IsValid(World) == false)
	{
		return TEXT("Unknown");
	}

	switch (World->GetNetMode())
	{
	case NM_ListenServer:
	case NM_DedicatedServer:
		return TEXT("Server");
	case NM_Client:
		return TEXT("Client");
	case NM_Standalone:
		return TEXT("Standalone");
	default:
		return TEXT("Unknown");
	}
}
