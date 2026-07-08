// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/REGameplayAbility.h"

UREGameplayAbility::UREGameplayAbility()
{
	// 액터당 인스턴스 1개. 맴버 변수에 상태 저장 가능
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
