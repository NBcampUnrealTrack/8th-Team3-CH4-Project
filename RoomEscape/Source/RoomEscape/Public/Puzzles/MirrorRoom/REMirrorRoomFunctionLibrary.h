#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorRoomFunctionLibrary.generated.h"

UCLASS()
class ROOMESCAPE_API UREMirrorRoomFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	static FString MirrorText(const FString& Source, EREMirrorAxis MirrorAxis);

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	static int32 MirrorClockHour(int32 ActualHour, EREMirrorAxis MirrorAxis);

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	static int32 NormalizeClockHour(int32 Hour);

	UFUNCTION(BlueprintPure, Category = "Mirror Room")
	static FText GetMirrorAxisText(EREMirrorAxis MirrorAxis);
};
