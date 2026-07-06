// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Flashlight.h"

#include "Character/REPlayerCharacter.h"

UGA_Flashlight::UGA_Flashlight()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UGA_Flashlight::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AREPlayerCharacter* Char = Cast<AREPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		Char->ServerToggleFlashlight();
	}
	
	// 토글 요청만 보내고 끝나는 어빌리티
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
