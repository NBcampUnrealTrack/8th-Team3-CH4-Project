// Fill out your copyright notice in the Description page of Project Settings.

#include "Puzzles/PianoSequence/REPianoBoard.h"
#include "Puzzles/PianoSequence/REPianoSequenceManager.h"
#include "Puzzles/PianoSequence/REPianoPatternData.h"
#include "UI/REPianoBoardWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Game/RENotifySubsystem.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "UI/LocalWidgetManager.h"
#include "UI/RERootCanvasWidget.h"

AREPianoBoard::AREPianoBoard()
{
}

void AREPianoBoard::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		if (AREPianoSequenceManager* Manager = GetPianoManager())
		{
			Manager->RegisterBoard(this);
			GenerateKeyPermutationOnce(Manager->GetPatternData());
		}
	}
}

void AREPianoBoard::GenerateKeyPermutationOnce(const UREPianoPatternData* InPatternData)
{
	if (IsValid(InPatternData) == false || KeyPermutation.Num() > 0)
	{
		return;
	}

	KeyPermutation.Reserve(InPatternData->NoteScale.Num());

	for (const FREPianoNote& Note : InPatternData->NoteScale)
	{
		KeyPermutation.Add(Note.NoteIndex);
	}

	// Fisher-Yates 셔플 - 최초 1회만 실행. 이후 오답이 나와도 다시 실행되지 않는다
	for (int32 Index = KeyPermutation.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		KeyPermutation.Swap(Index, SwapIndex);
	}

#if !UE_BUILD_SHIPPING
	// 디버그 전용 - 셔플된 슬롯-음 매핑을 로그에 한 번 출력 (서버에서만 실행됨)
	FString MapStr;
	for (int32 Slot = 0; Slot < KeyPermutation.Num(); ++Slot)
	{
		const int32 NoteIndex = KeyPermutation[Slot];
		FString Label = FString::FromInt(NoteIndex);
		if (const FREPianoNote* Note = InPatternData->FindNote(NoteIndex))
		{
			if (Note->NoteLabel.IsEmpty() == false)
			{
				Label = Note->NoteLabel.ToString();
			}
		}
		MapStr += FString::Printf(TEXT("[슬롯 %d = %s] "), Slot, *Label);
	}
	if (URENotifySubsystem* Notify = URENotifySubsystem::GetInstance(this))
	{
		Notify->NotifyEvent(RETag::Event::Debug::Piano, FString::Printf(TEXT("슬롯-음 매핑: %s"), *MapStr));
	}
#endif
}

int32 AREPianoBoard::GetSlotIndexForNote(int32 NoteIndex) const
{
	return KeyPermutation.IndexOfByKey(NoteIndex);
}

void AREPianoBoard::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);

	if (IsValid(PlayerController) == false || IsValid(PianoWidgetClass) == false)
	{
		return;
	}

	// 이 피아노를 상호작용한 클라이언트에게만 위젯을 열어주기 위해 소유권을 넘긴다 (REPuzzleHintActor와 동일 패턴)
	SetOwner(PlayerController);

	if (PlayerController->IsLocalController() == true)
	{
		OpenPianoWidgetLocal(PlayerController);
		return;
	}

	ClientOpenPianoWidget(PlayerController);
}

void AREPianoBoard::ClientOpenPianoWidget_Implementation(APlayerController* OwningPlayerController)
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(OwningPlayerController);
	OpenPianoWidgetLocal(LocalPlayerController);
}

void AREPianoBoard::OpenPianoWidgetLocal(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(PianoWidgetClass) == false)
	{
		return;
	}

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	URERootCanvasWidget* RootCanvas = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvas) == false)
	{
		return;
	}

	ActivePianoWidget = Cast<UREPianoBoardWidget>(RootCanvas->ShowGameplayWidget(PianoWidgetClass));
	//if (IsValid(ActivePianoWidget) == false)
	//{
	//	ActivePianoWidget = CreateWidget<UREPianoBoardWidget>(PlayerController, PianoWidgetClass);
	//}

	if (IsValid(ActivePianoWidget) == false)
	{
		return;
	}

	ActivePianoWidget->InitializeBoard(this);

	//if (ActivePianoWidget->IsInViewport() == false)
	//{
	//	ActivePianoWidget->AddToViewport();
	//}
}

void AREPianoBoard::RequestSubmitKeySlot(int32 SlotIndex)
{
	ServerSubmitKeySlot(SlotIndex);
}

void AREPianoBoard::ServerSubmitKeySlot_Implementation(int32 SlotIndex)
{
	if (KeyPermutation.IsValidIndex(SlotIndex) == false)
	{
		return;
	}

	const int32 NoteIndex = KeyPermutation[SlotIndex];
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	AActor* Interactor = IsValid(PlayerController) ? Cast<AActor>(PlayerController->GetPawn()) : nullptr;

	if (AREPianoSequenceManager* Manager = GetPianoManager())
	{
		Manager->SubmitKeyNote(this, NoteIndex, Interactor);
	}
}

void AREPianoBoard::PlayNote(int32 NoteIndex, FLinearColor GlowColor, USoundBase* NoteSound, float Duration)
{
	ReceiveNotePlayed(NoteIndex, GlowColor, NoteSound, Duration);
}

AREPianoSequenceManager* AREPianoBoard::GetPianoManager() const
{
	return Cast<AREPianoSequenceManager>(GetPuzzleManager());
}

APlayerController* AREPianoBoard::ResolvePlayerController(AActor* Actor) const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		return PlayerController;
	}

	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Cast<APlayerController>(Pawn->GetController());
	}

	return nullptr;
}

APlayerController* AREPianoBoard::ResolveLocalPlayerController(APlayerController* PreferredPlayerController) const
{
	if (IsValid(PreferredPlayerController) == true && PreferredPlayerController->IsLocalController() == true)
	{
		return PreferredPlayerController;
	}

	if (APlayerController* OwnerPlayerController = Cast<APlayerController>(GetOwner()))
	{
		if (OwnerPlayerController->IsLocalController() == true)
		{
			return OwnerPlayerController;
		}
	}

	return UGameplayStatics::GetPlayerController(this, 0);
}
