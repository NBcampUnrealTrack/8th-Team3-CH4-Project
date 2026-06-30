#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/REItemDataRow.h"
#include "REItemDataSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnREItemDataTableChangedSignature, UDataTable*, ItemDataTable);

UCLASS(BlueprintType)
class ROOMESCAPE_API UREItemDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Item")
	FOnREItemDataTableChangedSignature OnItemDataTableChanged;

public:
	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static UREItemDataSubsystem* GetInstance(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemDataTable(UDataTable* NewItemDataTable);

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UDataTable* GetItemDataTable() const { return ItemDataTable; }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsValidItemDataTable(const UDataTable* DataTable) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	bool HasItem(const FName& ItemId) const;

	const FREItemDataRow* FindItemData(const FName& ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	bool GetItemData(const FName& ItemId, FREItemDataRow& OutItemData) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetItemName(const FName& ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetItemDescription(const FName& ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	void GetAllItemIds(TArray<FName>& OutItemIds) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	void GetAllItemData(TArray<FName>& OutItemIds, TArray<FREItemDataRow>& OutItemDataList) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ItemDataTable;
};
