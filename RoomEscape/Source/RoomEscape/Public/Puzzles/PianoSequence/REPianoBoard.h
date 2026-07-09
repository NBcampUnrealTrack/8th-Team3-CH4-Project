// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REPianoBoard.generated.h"

class USoundBase;
class UREPianoPatternData;
class AREPianoSequenceManager;
class UREPianoBoardWidget;
class APlayerController;

// B방의 연주 피아노 - 상호작용하면 위젯이 열리고, 위젯 버튼(슬롯)을 누르면 서버로 제출된다.
// 슬롯 -> 실제 음 매핑(KeyPermutation)은 최초 1회만 셔플되고, 이후 오답이 나와도 바뀌지 않는다.
UCLASS(Blueprintable)
class ROOMESCAPE_API AREPianoBoard : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREPianoBoard();

	virtual void BeginPlay() override;

protected:
	// B방에서 열릴 연주 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano Sequence", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREPianoBoardWidget> PianoWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UREPianoBoardWidget> ActivePianoWidget;

public:
	// 위젯의 버튼 OnClicked에서 호출 - SlotIndex는 화면상 건반 위치(0..N-1)
	UFUNCTION(BlueprintCallable, Category = "Piano Sequence")
	void RequestSubmitKeySlot(int32 SlotIndex);

	// 디버그용 - 특정 음(NoteIndex)이 지금 몇 번 슬롯(버튼)에 매핑돼 있는지. 없으면 INDEX_NONE
	int32 GetSlotIndexForNote(int32 NoteIndex) const;

	// 눌린 슬롯의 실제 음 피드백 (Manager Multicast에서 호출됨)
	void PlayNote(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Piano Sequence")
	void ReceiveNotePlayed(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration);

protected:
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION(Client, Reliable)
	void ClientOpenPianoWidget(APlayerController* OwningPlayerController);

	UFUNCTION(Server, Reliable)
	void ServerSubmitKeySlot(int32 SlotIndex);

private:
	// 서버 전용 - 슬롯 인덱스 -> 실제 NoteIndex. 최초 1회만 채워지고 이후 변하지 않는다
	TArray<int32> KeyPermutation;

	void GenerateKeyPermutationOnce(const UREPianoPatternData* InPatternData);
	AREPianoSequenceManager* GetPianoManager() const;
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	APlayerController* ResolveLocalPlayerController(APlayerController* PreferredPlayerController) const;
	void OpenPianoWidgetLocal(APlayerController* PlayerController);
};
