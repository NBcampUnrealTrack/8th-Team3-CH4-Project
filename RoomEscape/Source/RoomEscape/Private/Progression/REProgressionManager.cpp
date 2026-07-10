// Fill out your copyright notice in the Description page of Project Settings.


#include "Progression/REProgressionManager.h"
#include "Progression/REProgressionDoor.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "Puzzles/Framework/REPuzzleResetPoint.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "Game/RENotifySubsystem.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "RoomEscape.h"

AREProgressionManager::AREProgressionManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	bAlwaysRelevant = true; // 진행 상태는 거리와 무관하게 항상 복제

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void AREProgressionManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentStage);
}

void AREProgressionManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		return;
	}

	// 테스트 레벨용: 2부부터 시작하도록 지정된 경우 (탈출 퍼즐 활성화는 레벨 인스턴스의 bStartActive에 맡김)
	if (StartStage != EREGameStage::RoomPhase)
	{
		CurrentStage = StartStage;
	}

	for (const TObjectPtr<AREPuzzleManager>& Puzzle : RoomPuzzles)
	{
		if (IsValid(Puzzle) == true)
		{
			Puzzle->OnPuzzleSolved.AddDynamic(this, &ThisClass::HandleRoomPuzzleSolved);
		}
	}

	if (IsValid(BombManager) == true)
	{
		BombManager->OnPuzzleSolved.AddDynamic(this, &ThisClass::HandleBombSolved);
		BombManager->OnPuzzleFailed.AddDynamic(this, &ThisClass::HandleBombFailed);
		// 배드엔딩(텔레포트·리셋·재시작)은 이 매니저가 단일 소유 — 폭탄 내부 자체 흐름은 끈다. (7/10 회의 결정)
		BombManager->SetBadEndingFlowDelegated(true);
	}

	if (RoomPuzzles.IsEmpty() == true)
	{
		UE_LOG(LogREEvent, Warning, TEXT("[Progression] RoomPuzzles가 비어 있음 - 배치 인스턴스 배선을 확인하세요."));
	}
	if (IsValid(BombManager) == false)
	{
		UE_LOG(LogREEvent, Warning, TEXT("[Progression] BombManager 미지정 - 엔딩/배드엔딩 흐름이 동작하지 않습니다."));
	}
}

EREGameStage AREProgressionManager::GetCurrentStage() const
{
	return CurrentStage;
}

void AREProgressionManager::HandleRoomPuzzleSolved()
{
	if (CurrentStage != EREGameStage::RoomPhase)
	{
		return;
	}

	NotifyProgress(RETag::Event::Progress::StageChanged.GetTag(),
		FString::Printf(TEXT("방 퍼즐 클리어 감지 (전체 클리어: %s)"), AreAllRoomPuzzlesSolved() ? TEXT("예") : TEXT("아니오")));

	if (AreAllRoomPuzzlesSolved() == true)
	{
		EnterEscapePhase();
	}
}

void AREProgressionManager::HandleBombSolved()
{
	if (CurrentStage != EREGameStage::EscapePhase)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(BadEndingTimerHandle);
	SetDoorsOpen(EscapeDoors, true);
	SetStage(EREGameStage::Escaped);
	NotifyProgress(RETag::Event::Progress::Cleared.GetTag(), TEXT("폭탄 해체 성공 - B방 지하 문 개방, 엔딩"));
}

void AREProgressionManager::HandleBombFailed()
{
	if (CurrentStage != EREGameStage::EscapePhase)
	{
		return;
	}

	NotifyProgress(RETag::Event::Progress::BadEnding.GetTag(),
		FString::Printf(TEXT("폭탄 폭발 - %.1f초 후 탈출 시퀀스 재시작"), BadEndingDelaySeconds));
	MulticastBadEndingStarted();

	if (BadEndingDelaySeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(BadEndingTimerHandle, this, &ThisClass::RestartEscapePhase, BadEndingDelaySeconds, false);
	}
	else
	{
		RestartEscapePhase();
	}
}

void AREProgressionManager::EnterEscapePhase()
{
	SetDoorsOpen(BasementDoors, true);
	ResetAndActivateEscapePuzzles();
	SetStage(EREGameStage::EscapePhase);
	NotifyProgress(RETag::Event::Progress::StageChanged.GetTag(), TEXT("방 퍼즐 전부 클리어 - 지하 문 개방, 탈출 시퀀스 시작"));
}

void AREProgressionManager::RestartEscapePhase()
{
	if (CurrentStage != EREGameStage::EscapePhase)
	{
		return;
	}

	TeleportPlayersToRestartPoints();
	ResetAndActivateEscapePuzzles();
	MulticastEscapePhaseRestarted();
	NotifyProgress(RETag::Event::Progress::Restarted.GetTag(), TEXT("탈출 시퀀스 재시작 (방 퍼즐 클리어 상태 유지)"));
}

void AREProgressionManager::ResetAndActivateEscapePuzzles()
{
	for (AREPuzzleManager* Puzzle : GetEscapePuzzlesIncludingBomb())
	{
		Puzzle->ResetToLocked();
		Puzzle->ActivatePuzzle();
	}
}

void AREProgressionManager::TeleportPlayersToRestartPoints()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;
		if (IsValid(Pawn) == false)
		{
			continue;
		}

		// 리슨 서버: 호스트 컨트롤러만 서버에서 로컬. 기본 가정은 호스트 = Player A(지하 진입자).
		const bool bIsPlayerA = (PC->IsLocalController() == bHostIsPlayerA);
		const AREPuzzleResetPoint* Point = bIsPlayerA ? RestartPointA.Get() : RestartPointB.Get();
		if (IsValid(Point) == false)
		{
			UE_LOG(LogREEvent, Warning, TEXT("[Progression] RestartPoint%s 미지정 - 텔레포트 생략"), bIsPlayerA ? TEXT("A") : TEXT("B"));
			continue;
		}

		Pawn->TeleportTo(Point->GetActorLocation(), Point->GetActorRotation());
		PC->SetControlRotation(Point->GetActorRotation());
	}
}

void AREProgressionManager::SetDoorsOpen(const TArray<TObjectPtr<AREProgressionDoor>>& Doors, bool bOpen)
{
	for (const TObjectPtr<AREProgressionDoor>& Door : Doors)
	{
		if (IsValid(Door) == true)
		{
			Door->SetOpen(bOpen);
		}
	}
}

TArray<AREPuzzleManager*> AREProgressionManager::GetEscapePuzzlesIncludingBomb() const
{
	TArray<AREPuzzleManager*> Result;
	for (const TObjectPtr<AREPuzzleManager>& Puzzle : EscapePuzzles)
	{
		if (IsValid(Puzzle) == true)
		{
			Result.AddUnique(Puzzle.Get());
		}
	}
	if (IsValid(BombManager) == true)
	{
		Result.AddUnique(BombManager.Get());
	}
	return Result;
}

bool AREProgressionManager::AreAllRoomPuzzlesSolved() const
{
	for (const TObjectPtr<AREPuzzleManager>& Puzzle : RoomPuzzles)
	{
		if (IsValid(Puzzle) == true && Puzzle->IsSolved() == false)
		{
			return false;
		}
	}
	return true;
}

void AREProgressionManager::SetStage(EREGameStage NewStage)
{
	if (HasAuthority() == false || CurrentStage == NewStage)
	{
		return;
	}

	CurrentStage = NewStage;
	OnRep_CurrentStage();
}

void AREProgressionManager::OnRep_CurrentStage()
{
	OnStageChanged.Broadcast(CurrentStage);
	ReceiveStageChanged(CurrentStage);
}

void AREProgressionManager::MulticastBadEndingStarted_Implementation()
{
	OnBadEndingStarted.Broadcast();
}

void AREProgressionManager::MulticastEscapePhaseRestarted_Implementation()
{
	OnEscapePhaseRestarted.Broadcast();
}

void AREProgressionManager::NotifyProgress(const FGameplayTag& EventTag, const FString& Message) const
{
	if (URENotifySubsystem* Notify = URENotifySubsystem::GetInstance(this))
	{
		Notify->NotifyEvent(EventTag, Message);
	}
}
