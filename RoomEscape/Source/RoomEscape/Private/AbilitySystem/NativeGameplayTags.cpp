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