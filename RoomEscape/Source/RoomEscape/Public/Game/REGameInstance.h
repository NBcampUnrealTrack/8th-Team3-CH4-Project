#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "REGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "NetWork")
	FString ServerName;
	
	UPROPERTY(BlueprintReadOnly, Category = "NetWork")
	int32 Ping;
	
	UPROPERTY(BlueprintReadOnly, Category = "NetWork")
	int32 SearchIndex;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchCompleted, const TArray<FServerInfo>&, ServerList);

UCLASS()
class ROOMESCAPE_API UREGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UREGameInstance();
	
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HostGame(FString RoomName);
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void FindGames();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinSelectedGame(int32 SessionIndex);
	
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnSearchCompleted OnSearchCompleted;
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDataTable(UDataTable* NewItemDataTable);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true", RequiredAssetDataTags = "RowStructure=/Script/RoomEscape.REItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;

	TWeakPtr<IOnlineSession> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	// Delegate Callback Function
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
