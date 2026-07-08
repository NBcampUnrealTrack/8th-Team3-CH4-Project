#include "Game/REGameInstance.h"
#include "Data/REItemDataSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

UREGameInstance::UREGameInstance()
{
}

void UREGameInstance::Init()
{
	Super::Init();

	SetItemDataTable(ItemDataTable);
	
	// Get Null to connect session interface
	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		SessionInterface = Subsystem->GetSessionInterface();
		
		if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
		{
			SharedSession->OnCreateSessionCompleteDelegates.AddUObject(this, &UREGameInstance::OnCreateSessionComplete);
			SharedSession->OnFindSessionsCompleteDelegates.AddUObject(this, &UREGameInstance::OnFindSessionsComplete);
			SharedSession->OnJoinSessionCompleteDelegates.AddUObject(this, &UREGameInstance::OnJoinSessionComplete);
		}
	}
}

void UREGameInstance::SetItemDataTable(UDataTable* NewItemDataTable)
{
	ItemDataTable = NewItemDataTable;

	if (UREItemDataSubsystem* ItemDataSubsystem = GetSubsystem<UREItemDataSubsystem>())
	{
		ItemDataSubsystem->SetItemDataTable(ItemDataTable);
	}
}


// -- 방 만들기 --

void UREGameInstance::HostGame(FString RoomName, FString Password)
{
	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;			// Steam 미 사용시 LAN 사용
		SessionSettings.NumPublicConnections = 2;	// 2인 제한
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		
		SessionSettings.Set(FName("SERVER_NAME"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(FName("ROOM_PASSWORD"), Password, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		SharedSession->CreateSession(0, FName("RoomEscapeSession"), SessionSettings);
	}
}

void UREGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel("/Game/Level/MainLevel?listen");
	}
}

// -- 방 찾기 --

void UREGameInstance::JoinGame(FString InputRoomName, FString InputPassword)
{
	TargetRoomName = InputRoomName;
	TargetPassword = InputPassword;

	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = true;       
		SessionSearch->MaxSearchResults = 100;
       
		SharedSession->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UREGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
		{
			for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
			{
				FString FoundName;
				FString FoundPassword;

				Result.Session.SessionSettings.Get(FName("SERVER_NAME"), FoundName);
				Result.Session.SessionSettings.Get(FName("ROOM_PASSWORD"), FoundPassword);

				if (FoundName == TargetRoomName && FoundPassword == TargetPassword)
				{
					SharedSession->JoinSession(0, FName("RoomEscapeSession"), Result);
					return; 
				}
			}
		}
	}

	OnJoinProcessResult.Broadcast(false, TEXT("방 이름이나 비밀번호가 일치하지 않습니다."));
}

void UREGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
		{
			FString JoinAddress;
          
			if (SharedSession->GetResolvedConnectString(SessionName, JoinAddress))
			{
				OnJoinProcessResult.Broadcast(true, TEXT("접속 성공! 게임으로 이동합니다."));

				if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
	else
	{
		OnJoinProcessResult.Broadcast(false, TEXT("접속에 실패했습니다."));
	}
}