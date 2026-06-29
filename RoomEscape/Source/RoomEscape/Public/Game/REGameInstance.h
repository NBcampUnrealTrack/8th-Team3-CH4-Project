#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "REGameInstance.generated.h"

UCLASS()
class ROOMESCAPE_API UREGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UREGameInstance();
	
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HostGame();
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinGame();
	
protected:
	TWeakPtr<IOnlineSession> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	// Delegate Callback Function
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
