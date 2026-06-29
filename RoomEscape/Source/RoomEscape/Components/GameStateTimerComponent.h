// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WidgetInitializableInterface.h"
#include "GameStateTimerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTimerChangeSignature, const FTimespan&, RemainTime, const FTimespan&, MaxTime);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOMESCAPE_API UGameStateTimerComponent : public UActorComponent, public IWidgetInitializableInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGameStateTimerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// IWidgetInitializableInterface을(를) 통해 상속됨
	void InitWidget_Implementation() override;

public:
	UPROPERTY(BlueprintAssignable)
	FTimerChangeSignature OnTimerValueChanged;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	AGameStateBase* OwnerGameStateBase;

	// 타이머의 Widget 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> TimerWidgetClass;

	// 최대 제한시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_MaxTime)
	FTimespan MaxTime;

	// 현재 남은 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_RemainTime)
	FTimespan RemainTime;

	// 제한 시간을 일정 시간 단위로 감소시키는 TimerHandle
	UPROPERTY()
	FTimerHandle TickTimer;

public:
	// 제한 시간을 초기화하는 함수
	// 현재 남은 시간을 초기값으로 설정
	UFUNCTION(BlueprintCallable)
	void InitializeTimer();

	// 타이머를 시작하는 함수
	UFUNCTION(BlueprintCallable)
	void StartTimer();

	// 타이머를 일시 정지하는 함수
	UFUNCTION(BlueprintCallable)
	void PauseTimer();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FTimespan GetMaxTime() const { return MaxTime; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FTimespan GetRemainTime() const { return RemainTime; }

	UFUNCTION(BlueprintAuthorityOnly)
	void SetMaxTime(const FTimespan& NewMaxTime);

	// Debug 전용
	UFUNCTION(BlueprintAuthorityOnly)
	void SetRemainTime(const FTimespan& NewRemainTime);
	
protected:
	UFUNCTION()
	void DecreaseRemainTime(double DeltaSeconds);

	UFUNCTION()
	FORCEINLINE void OnRep_MaxTime() const { OnTimerValueChanged.Broadcast(RemainTime, MaxTime); }

	UFUNCTION()
	FORCEINLINE void OnRep_RemainTime() const { OnTimerValueChanged.Broadcast(RemainTime, MaxTime); }
};
