// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_Interact.h"

#include "AbilitySystem/NativeGameplayTags.h"
#include "Character/REPlayerCharacter.h"
#include "Game/RENotifySubsystem.h"
#include "Interaction/REInteractable.h"

UGA_Interact::UGA_Interact()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (AREPlayerCharacter* Char = Cast<AREPlayerCharacter>(Avatar))
	{
		FHitResult Hit;
		AActor* Target = Char->TraceForInteractable(Hit);   // 클라의 정확한 카메라로 트레이스

		if (Target && Target->Implements<UREInteractable>())
		{
			Char->ServerInteract(Target);   // 대상만 서버로 전달

			if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(Char))
			{
				NotifySubsystem->NotifyEvent(RETag::Event::Interact::Succeeded,
					FString::Printf(TEXT("%s이(가) %s와(과) 상호작용했습니다"), *Char->GetName(), *Target->GetName()));
			}
		}
	}

	// 한 번 실행하고 끝나는 어빌리티 → 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo,true, false);
}
