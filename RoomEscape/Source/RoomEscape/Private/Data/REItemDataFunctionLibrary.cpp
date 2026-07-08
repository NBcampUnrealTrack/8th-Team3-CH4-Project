#include "Data/REItemDataFunctionLibrary.h"
#include "Data/REItemDataSubsystem.h"

UREItemDataSubsystem* UREItemDataFunctionLibrary::GetItemDataSubsystem(const UObject* WorldContextObject)
{
	return UREItemDataSubsystem::GetInstance(WorldContextObject);
}

bool UREItemDataFunctionLibrary::HasItem(const UObject* WorldContextObject, const FName& ItemId)
{
	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	return IsValid(ItemDataSubsystem) == true && ItemDataSubsystem->HasItem(ItemId) == true;
}

bool UREItemDataFunctionLibrary::GetItemData(const UObject* WorldContextObject, const FName& ItemId, FREItemDataRow& OutItemData)
{
	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	return IsValid(ItemDataSubsystem) == true && ItemDataSubsystem->GetItemData(ItemId, OutItemData) == true;
}

FText UREItemDataFunctionLibrary::GetItemName(const UObject* WorldContextObject, const FName& ItemId)
{
	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	return IsValid(ItemDataSubsystem) == true ? ItemDataSubsystem->GetItemName(ItemId) : FText::GetEmpty();
}

FText UREItemDataFunctionLibrary::GetItemDescription(const UObject* WorldContextObject, const FName& ItemId)
{
	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	return IsValid(ItemDataSubsystem) == true ? ItemDataSubsystem->GetItemDescription(ItemId) : FText::GetEmpty();
}

void UREItemDataFunctionLibrary::GetAllItemIds(const UObject* WorldContextObject, TArray<FName>& OutItemIds)
{
	OutItemIds.Reset();

	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	if (IsValid(ItemDataSubsystem) == false)
	{
		return;
	}

	ItemDataSubsystem->GetAllItemIds(OutItemIds);
}

void UREItemDataFunctionLibrary::GetAllItemData(const UObject* WorldContextObject, TArray<FName>& OutItemIds, TArray<FREItemDataRow>& OutItemDataList)
{
	OutItemIds.Reset();
	OutItemDataList.Reset();

	const UREItemDataSubsystem* ItemDataSubsystem = GetItemDataSubsystem(WorldContextObject);
	if (IsValid(ItemDataSubsystem) == false)
	{
		return;
	}

	ItemDataSubsystem->GetAllItemData(OutItemIds, OutItemDataList);
}
