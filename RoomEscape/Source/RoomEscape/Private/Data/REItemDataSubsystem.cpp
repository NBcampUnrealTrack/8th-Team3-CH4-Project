#include "Data/REItemDataSubsystem.h"
#include "Engine/World.h"

UREItemDataSubsystem* UREItemDataSubsystem::GetInstance(const UObject* WorldContextObject)
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
	return IsValid(GameInstance) == true ? GameInstance->GetSubsystem<UREItemDataSubsystem>() : nullptr;
}

void UREItemDataSubsystem::SetItemDataTable(UDataTable* NewItemDataTable)
{
	if (ItemDataTable == NewItemDataTable)
	{
		return;
	}

	ItemDataTable = IsValidItemDataTable(NewItemDataTable) == true ? NewItemDataTable : nullptr;
	OnItemDataTableChanged.Broadcast(ItemDataTable);
}

bool UREItemDataSubsystem::IsValidItemDataTable(const UDataTable* DataTable) const
{
	return IsValid(DataTable) == true && IsValid(DataTable->GetRowStruct()) == true && DataTable->GetRowStruct()->IsChildOf(FREItemDataRow::StaticStruct()) == true;
}

bool UREItemDataSubsystem::HasItem(const FName& ItemId) const
{
	return FindItemData(ItemId) != nullptr;
}

const FREItemDataRow* UREItemDataSubsystem::FindItemData(const FName& ItemId) const
{
	if (ItemId.IsNone() == true || IsValidItemDataTable(ItemDataTable) == false)
	{
		return nullptr;
	}

	return ItemDataTable->FindRow<FREItemDataRow>(ItemId, TEXT("ItemData"), false);
}

bool UREItemDataSubsystem::GetItemData(const FName& ItemId, FREItemDataRow& OutItemData) const
{
	const FREItemDataRow* ItemData = FindItemData(ItemId);
	if (ItemData == nullptr)
	{
		return false;
	}

	OutItemData = *ItemData;
	return true;
}

FText UREItemDataSubsystem::GetItemName(const FName& ItemId) const
{
	const FREItemDataRow* ItemData = FindItemData(ItemId);
	return ItemData != nullptr ? ItemData->ItemName : FText::GetEmpty();
}

FText UREItemDataSubsystem::GetItemDescription(const FName& ItemId) const
{
	const FREItemDataRow* ItemData = FindItemData(ItemId);
	return ItemData != nullptr ? ItemData->Description : FText::GetEmpty();
}

void UREItemDataSubsystem::GetAllItemIds(TArray<FName>& OutItemIds) const
{
	OutItemIds.Reset();

	if (IsValidItemDataTable(ItemDataTable) == false)
	{
		return;
	}

	OutItemIds = ItemDataTable->GetRowNames();
}

void UREItemDataSubsystem::GetAllItemData(TArray<FName>& OutItemIds, TArray<FREItemDataRow>& OutItemDataList) const
{
	OutItemIds.Reset();
	OutItemDataList.Reset();

	if (IsValidItemDataTable(ItemDataTable) == false)
	{
		return;
	}

	const TArray<FName> RowNames = ItemDataTable->GetRowNames();
	OutItemIds.Reserve(RowNames.Num());
	OutItemDataList.Reserve(RowNames.Num());

	for (const FName& RowName : RowNames)
	{
		const FREItemDataRow* ItemData = FindItemData(RowName);
		if (ItemData == nullptr)
		{
			continue;
		}

		OutItemIds.Add(RowName);
		OutItemDataList.Add(*ItemData);
	}
}
