// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NativeGameplayTags.h"

namespace RETag
{
	namespace Input
	{
		UE_DEFINE_GAMEPLAY_TAG(Interact, "Input.Interact");
	}
	
	namespace Ability
	{
		UE_DEFINE_GAMEPLAY_TAG(Interact, "Ability.Interact");
	}
	
	namespace Event
	{
		namespace Interact
		{
			UE_DEFINE_GAMEPLAY_TAG(Succeeded, "Event.Interact.Succeeded");
		}

		namespace Timer
		{
			UE_DEFINE_GAMEPLAY_TAG(AlmostOver, "Event.Timer.AlmostOver");
		}
		
		namespace Item
		{
			UE_DEFINE_GAMEPLAY_TAG(PickedUp, "Event.Item.PickedUp");
		}

		namespace Puzzle
		{
			UE_DEFINE_GAMEPLAY_TAG(Solved, "Event.Puzzle.Solved");
			UE_DEFINE_GAMEPLAY_TAG(Failed, "Event.Puzzle.Failed");
			UE_DEFINE_GAMEPLAY_TAG(TrapTriggered, "Event.Puzzle.TrapTriggered");
		}

		namespace Debug
		{
			UE_DEFINE_GAMEPLAY_TAG(Test, "Event.Debug.Test");
			UE_DEFINE_GAMEPLAY_TAG(Piano, "Event.Debug.Piano");
		}

		namespace Progress
		{
			UE_DEFINE_GAMEPLAY_TAG(StageChanged, "Event.Progress.StageChanged");
			UE_DEFINE_GAMEPLAY_TAG(BadEnding, "Event.Progress.BadEnding");
			UE_DEFINE_GAMEPLAY_TAG(Restarted, "Event.Progress.Restarted");
			UE_DEFINE_GAMEPLAY_TAG(Cleared, "Event.Progress.Cleared");
		}
	}

	namespace State
	{
		UE_DEFINE_GAMEPLAY_TAG(Interacting, "State.Interacting");
		
		namespace Flashlight
		{
			UE_DEFINE_GAMEPLAY_TAG(On, "State.Flashlight.On");
		}
		
		namespace Holding
		{
			UE_DEFINE_GAMEPLAY_TAG(Radio, "State.Holding.Radio");
		}
	}
}