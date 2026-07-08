// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "REGameplayAbility.h"
#include "GA_Flashlight.generated.h"

/**
 * 손전등 토글 어빌리티.
 */
UCLASS()
class ROOMESCAPE_API UGA_Flashlight : public UREGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Flashlight();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
};
