// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/GlassMaze/GlassMazeManager.h"

#include "AbilitySystem/NativeGameplayTags.h"
#include "Character/REPlayerCharacter.h"
#include "Game/RENotifySubsystem.h"
#include "Puzzles/GlassMaze/GlassWall.h"
#include "Puzzles/GlassMaze/MazeLever.h"
#include "Puzzles/GlassMaze/MazeTrapTile.h"
#include "RoomEscape.h"

void AGlassMazeManager::HandlePuzzleSolved()
{
	Super::HandlePuzzleSolved();

	// MarkSolved 경로 = 서버에서만 호출됨
	if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Puzzle::Solved,
			FString::Printf(TEXT("%s 퍼즐 클리어"), *GetName()));
	}
}

void AGlassMazeManager::HandlePuzzleLocked()
{
	Super::HandlePuzzleLocked();

	// ResetToLocked 경로 = 서버에서만 호출됨. 진행 매니저가 텔레포트 후에 리셋하므로 벽에 플레이어가 겹칠 일은 없음
	for (AGlassWall* Wall : OpenableWalls)
	{
		if (Wall && Wall->IsOpen() && Wall->SetOpen(false) == false)
		{
			UE_LOG(LogREEvent, Warning, TEXT("[GlassMaze] %s 닫기 실패(플레이어 겹침) - 리셋 후에도 열린 상태로 남음"), *Wall->GetName());
		}
	}

	for (AMazeLever* Lever : Levers)
	{
		if (Lever)
		{
			Lever->ResetLeverState();
		}
	}
}

void AGlassMazeManager::RegisterOpenableWall(AGlassWall* Wall)
{
	if (Wall)
	{
		OpenableWalls.AddUnique(Wall);
	}
}

void AGlassMazeManager::RegisterLever(AMazeLever* Lever)
{
	if (Lever)
	{
		Levers.AddUnique(Lever);
	}
}

void AGlassMazeManager::SetWallsOpenByLever(FName LeverID, bool bOpen)
{
	if (!HasAuthority() || !IsActive() || LeverID.IsNone())
	{
		return;
	}

	for (AGlassWall* Wall : OpenableWalls)
	{
		if (Wall && Wall->LeverID == LeverID)
		{
			Wall->SetOpen(bOpen);	// 닫기는 플레이어가 겹쳐 있으면 벽이 스스로 거부
		}
	}
}

void AGlassMazeManager::HandleTrapTriggered(AMazeTrapTile* Trap, AREPlayerCharacter* Victim)
{
	if (!HasAuthority() || !IsActive() || !Victim || !Trap)
	{
		return;
	}

	Trap->Multicast_OnTriggered();

	if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Puzzle::TrapTriggered,
			FString::Printf(TEXT("%s이(가) 함정을 밟았습니다"), *Victim->GetName()));
	}
}

void AGlassMazeManager::HandleExitReached(AActor* Reacher)
{
	if (!HasAuthority() || !IsActive())
	{
		return;
	}
	MarkSolved();
}
