// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/GameplayTags/Public/NativeGameplayTags.h"

/**
 * 프로젝트 전역 네이티브 게임플레이 태그.
 * 사용 예 : RETag::State_Flashlight_On
 */
namespace RETag
{
	// --- 입력 식별 (입력 -> 어빌리티, 지금은 예비)
	namespace Input
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact);	
	}
	
	// -- 어빌리티 식별
	namespace Ability
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact);	
	}
	
	// --- 게임 이벤트 알림 식별 (URENotifySubsystem::NotifyEvent에 전달)
	namespace Event
	{
		namespace Interact
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Succeeded);
		}

		namespace Timer
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(AlmostOver);
		}

		// 아직 구현 전인 시스템의 이벤트도 태그만 미리 등록
		namespace Puzzle
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Solved);
		}

		namespace Debug
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Test);
		}
	}

	// --- 상태 (수치 대신 상태를 태그로)
	namespace State
	{
		// 밑 부분은 예시입니다 지우셔도 됩니다.
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interacting);

		namespace Flashlight
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(On);	
		}

		namespace Holding
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Radio);	
		}
	}
}