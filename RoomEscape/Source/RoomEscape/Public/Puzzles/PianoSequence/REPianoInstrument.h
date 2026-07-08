// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REPianoInstrument.generated.h"

class USoundBase;
class AREPianoSequenceManager;

// A방의 피아노 - 상호작용할 때마다 매니저에게 멜로디 재생을 요청한다
UCLASS(Blueprintable)
class ROOMESCAPE_API AREPianoInstrument : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREPianoInstrument();

	virtual void BeginPlay() override;

	// 매니저가 멜로디를 재생할 때 호출 (Multicast에서 호출됨)
	void PlayNote(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration);

	// 실제 발광/사운드 연출은 BP에서 구현
	UFUNCTION(BlueprintImplementableEvent, Category = "Piano Sequence")
	void ReceiveNotePlayed(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration);

protected:
	virtual void HandleInteract(AActor* Interactor) override;

private:
	AREPianoSequenceManager* GetPianoManager() const;
};
