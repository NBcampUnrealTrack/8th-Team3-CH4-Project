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

void UREGameInstance::HostGame(FString RoomName)
{
	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;			// Steam 미 사용시 LAN 사용
		SessionSettings.NumPublicConnections = 2;	// 2인 제한
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		
		SessionSettings.Set(FName("SERVER_NAME"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
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

void UREGameInstance::FindGames()
{
	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = true;			// LAN 환경에서 검색
		SessionSearch->MaxSearchResults = 100;
		
		SharedSession->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UREGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	TArray<FServerInfo> FoundServers;
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[i];
			FServerInfo NewServer;
			NewServer.SearchIndex = i;
			NewServer.Ping = Result.PingInMs;
			
			FString FoundName;
			
			if (Result.Session.SessionSettings.Get(FName("SERVER_NAME"), FoundName))
			{
				NewServer.ServerName = FoundName;
			}
			else
			{
				NewServer.ServerName = TEXT("이름 없는 방");
			}
			
			FoundServers.Add(NewServer);
		}
	}
	
	OnSearchCompleted.Broadcast(FoundServers);
}

void UREGameInstance::JoinSelectedGame(int32 SessionIndex)
{
	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		if (SessionSearch.IsValid() && SessionSearch->SearchResults.IsValidIndex(SessionIndex))
		{
			SharedSession->JoinSession(0, FName("EscapeSession"), SessionSearch->SearchResults[SessionIndex]);
		}
	}
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
				// 주소를 성공적으로 획득했다면 해당 주소의 서버로 클라이언트 Travel
				if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
}
