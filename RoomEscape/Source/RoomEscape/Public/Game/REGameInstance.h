#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "REGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinProcessResult, bool, bSuccess, const FString&, Message);

UCLASS()
class ROOMESCAPE_API UREGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UREGameInstance();
	
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HostGame(FString RoomName, FString Password);
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinGame(FString InputRoomName, FString InputPassword);
	
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnJoinProcessResult OnJoinProcessResult;
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDataTable(UDataTable* NewItemDataTable);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true", RequiredAssetDataTags = "RowStructure=/Script/RoomEscape.REItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;

	TWeakPtr<IOnlineSession> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	FString TargetRoomName;
	FString TargetPassword;
	
	// Delegate Callback Function
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
