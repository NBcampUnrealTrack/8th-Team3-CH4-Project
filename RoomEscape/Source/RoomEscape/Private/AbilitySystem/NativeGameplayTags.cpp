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

	// --- UI
	namespace Widget
	{
		// 기초 레이아웃 (Root Canvas)
		UE_DEFINE_GAMEPLAY_TAG(RootLayout, "UI.RootLayout");

		UE_DEFINE_GAMEPLAY_TAG(PrimaryLayer, "UI.Layer.Primary");

		UE_DEFINE_GAMEPLAY_TAG(GameplayLayer, "UI.Layer.Gameplay");

		UE_DEFINE_GAMEPLAY_TAG(OverlayLayer, "UI.Layer.Overlay");

		UE_DEFINE_GAMEPLAY_TAG(PopupLayer, "UI.Layer.Popup");

		UE_DEFINE_GAMEPLAY_TAG(VisualEffectLayer, "UI.Layer.VisualEffect");
	}
}