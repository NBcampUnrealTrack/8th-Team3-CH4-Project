#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/RESessionPlayerStateComponent.h"
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
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void LeaveGame();
	
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnJoinProcessResult OnJoinProcessResult;
	
	// 로비에서 확정된 호스트의 역할. None이면 로비를 거치지 않은 실행(테스트 레벨)
	UFUNCTION(BlueprintPure, Category = "Session Room")
	FORCEINLINE ERESpawnRoomType GetHostSpawnRoomType() const { return HostSpawnRoomType; }

	void SetHostSpawnRoomType(ERESpawnRoomType InRoomType) { HostSpawnRoomType = InRoomType; }

	// Alpha = Player A(지하 진입자) 매핑. None이면 기본값 true(기존 동작 유지)
	bool IsHostPlayerA() const { return HostSpawnRoomType != ERESpawnRoomType::Beta; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDataTable(UDataTable* NewItemDataTable);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true", RequiredAssetDataTags = "RowStructure=/Script/RoomEscape.REItemDataRow"))
	TObjectPtr<UDataTable> ItemDataTable;

	ERESpawnRoomType HostSpawnRoomType = ERESpawnRoomType::None;

	TWeakPtr<IOnlineSession> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	
	FString TargetRoomName;
	FString TargetPassword;
	
	// Delegate Callback Function
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};
