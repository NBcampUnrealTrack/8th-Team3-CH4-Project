// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Puzzles/PianoSequence/REPianoSequenceTypes.h"
#include "REPianoPatternData.generated.h"

// 피아노 퍼즐 설계 데이터 - 도레미파솔라시도 8음 + 멜로디 길이만 조절
UCLASS(BlueprintType)
class ROOMESCAPE_API UREPianoPatternData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 8음계
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	TArray<FREPianoNote> NoteScale;

	// 한 번에 재생 / 입력할 멜로디 길이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano", meta = (ClampMin = "1"))
	int32 MelodyLength = 4;

	// 각 음 재생 간격(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano", meta = (ClampMin = "0.1"))
	float ShowDuration = 0.6f;

	// C++ 전용 - USTRUCT 포인터는 UFUNCTION(Blueprint 노출)으로 리턴할 수 없어 리플렉션에서 제외
	const FREPianoNote* FindNote(int32 NoteIndex) const;

	UFUNCTION(BlueprintPure, Category = "Piano")
	bool IsPatternValid() const;
};
