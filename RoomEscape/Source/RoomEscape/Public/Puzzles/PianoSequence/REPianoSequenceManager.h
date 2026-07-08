// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "REPianoSequenceManager.generated.h"

class UREPianoPatternData;
class AREPianoInstrument;
class AREPianoBoard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREPianoAttemptResultSignature, bool, bCorrect);

UCLASS(Blueprintable)
class ROOMESCAPE_API AREPianoSequenceManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	AREPianoSequenceManager();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	TObjectPtr<UREPianoPatternData> PatternData;

	UPROPERTY(BlueprintAssignable, Category = "Piano")
	FREPianoAttemptResultSignature OnPianoAttemptResult;

	const UREPianoPatternData* GetPatternData() const { return PatternData; }

	// A방 피아노가 BeginPlay에서 자신을 등록한다
	void RegisterInstrument(AREPianoInstrument* InInstrument);

	// B방 피아노가 BeginPlay에서 자신을 등록한다
	void RegisterBoard(AREPianoBoard* InBoard);

	// A가 피아노에 상호작용 - 필요 시 새 멜로디를 뽑아 재생
	void RequestPlayMelody(AActor* Interactor);

	// B가 위젯 슬롯을 하나 눌렀을 때 제출 - NoteIndex는 이미 셔플이 반영된 실제 음
	void SubmitKeyNote(AREPianoBoard* InBoard, int32 NoteIndex, AActor* Interactor);

protected:
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;

private:
	// 서버 전용 - 클라이언트에 알려주지 않는다(정답 유출 방지)
	TArray<int32> CurrentMelody;
	TArray<int32> PressedBuffer;
	bool bNeedsNewMelody = true;
	bool bIsPlayingBack = false;
	int32 PlaybackNoteCursor = 0;
	FTimerHandle PlaybackTimerHandle;

	UPROPERTY()
	TObjectPtr<AREPianoInstrument> Instrument;

	UPROPERTY()
	TObjectPtr<AREPianoBoard> Board;

	void GenerateNewMelody();
	void PlayNextNoteInPlayback();
	void EvaluateAttempt(AActor* Interactor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayNote(int32 NoteIndex, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastKeyFeedback(int32 NoteIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttemptResult(bool bCorrect);

};
