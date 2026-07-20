// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "REPianoBoardWidget.generated.h"

class AREPianoBoard;

// B방 연주 위젯의 C++ 진입점 - 버튼 8개/레이아웃은 WBP에서 구현하고, 버튼 OnClicked에서
// BoardRef->RequestSubmitKeySlot(N)을 호출하면 된다
UCLASS(Blueprintable)
class ROOMESCAPE_API UREPianoBoardWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Piano Sequence")
	void InitializeBoard(AREPianoBoard* InBoard);

	UPROPERTY(BlueprintReadOnly, Category = "Piano Sequence")
	TObjectPtr<AREPianoBoard> BoardRef;
};
