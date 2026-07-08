#pragma once

#include "CoreMinimal.h"
#include "REPianoSequenceTypes.generated.h"

class USoundBase;

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREPianoNote
{
	GENERATED_BODY()
	
	// 이 음의 고유 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	int32 NoteIndex = 0;
	
	// 음계 표시용 라벨(디버그)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	FText NoteLabel;
	
	// 재생 / 입력 시 발광할 색상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	FLinearColor GlowColor = FLinearColor::White;
	
	// 재생 / 입력 시 재생할 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	TObjectPtr<USoundBase> NoteSound;
};
