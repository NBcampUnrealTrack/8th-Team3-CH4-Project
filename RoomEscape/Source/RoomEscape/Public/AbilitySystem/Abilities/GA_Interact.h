// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "REGameplayAbility.h"
#include "GA_Interact.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UGA_Interact : public UREGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Interact();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
};
