// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "REGameplayAbility.generated.h"

/**
 * 프로젝트 공통 어빌리티 베이스
 * 모든 GA는 이 클래스를 상속
 * 지금은 뼈대만 있음
 */
UCLASS()
class ROOMESCAPE_API UREGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UREGameplayAbility();
};
