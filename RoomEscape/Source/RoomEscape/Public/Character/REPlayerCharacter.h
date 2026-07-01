// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "REPlayerCharacter.generated.h"

struct FInputActionValue;
class UInputAction;
class UREGameplayAbility;
class UCameraComponent;
class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class ROOMESCAPE_API AREPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AREPlayerCharacter();
	
	// IAbilitySystemComponent 구현 - 외부(GAS, 다른 액터)가 이 캐릭터의 ASC를 얻는 표준 통로
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* Target);
	
	AActor* TraceForInteractable(FHitResult& OutHit) const;
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// 빙의 / 복제 훅 - ASC 초기화 진입점
	virtual void PossessedBy(AController* NewController) override;	// 서버
	virtual void OnRep_PlayerState() override;						// 원격 클라
	
	void Input_Interact();
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	
	// 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// 내 팔 / 손
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;
	
	// Input
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	// Interaction
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractionDistance = 250.0f;
	
	// GAS 코어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;
	
	// 시작 시  부여할 기본 어빌리티들
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TArray<TSubclassOf<UREGameplayAbility>> DefaultAbilities;
	
private:
	void InitAbilityActorInfo();	// 양쪽 공통 : ASC에 소유자/아바타 연결
	void GrantDefaultAbilities();	// 서버 전용 : 기본 어빌리티 부여
};
