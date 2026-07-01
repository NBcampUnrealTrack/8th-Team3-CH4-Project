#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "REItemDataRow.generated.h"

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = "true"))
	FText Description;
};
