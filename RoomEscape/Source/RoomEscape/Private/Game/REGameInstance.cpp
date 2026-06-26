#include "Game/REGameInstance.h"
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

void UREGameInstance::HostGame()
{
	if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;
		SessionSettings.NumPublicConnections = 2;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		
		SharedSession->CreateSession(0, FName("RoomEscapeSession"), SessionSettings);
	}
}

void UREGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel("/Game/Content/Level/MainLevel?listen");
	}
}

void UREGameInstance::JoinGame()
{
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
	if (bWasSuccessful && SessionSearch->SearchResults.Num() > 0)
	{
		if (TSharedPtr<IOnlineSession> SharedSession = SessionInterface.Pin())
		{
			SharedSession->JoinSession(0, FName("EscapeSession"), SessionSearch->SearchResults[0]);
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
				if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
}