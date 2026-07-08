#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/REItemDataRow.h"
#include "REItemDataFunctionLibrary.generated.h"

class UREItemDataSubsystem;

UCLASS()
class ROOMESCAPE_API UREItemDataFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static UREItemDataSubsystem* GetItemDataSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static bool HasItem(const UObject* WorldContextObject, const FName& ItemId);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static bool GetItemData(const UObject* WorldContextObject, const FName& ItemId, FREItemDataRow& OutItemData);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static FText GetItemName(const UObject* WorldContextObject, const FName& ItemId);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static FText GetItemDescription(const UObject* WorldContextObject, const FName& ItemId);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static void GetAllItemIds(const UObject* WorldContextObject, TArray<FName>& OutItemIds);

	UFUNCTION(BlueprintPure, Category = "Item", meta = (WorldContext = "WorldContextObject"))
	static void GetAllItemData(const UObject* WorldContextObject, TArray<FName>& OutItemIds, TArray<FREItemDataRow>& OutItemDataList);
};
