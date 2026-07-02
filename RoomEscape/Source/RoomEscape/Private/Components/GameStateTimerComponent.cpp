// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GameStateTimerComponent.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "Game/RENotifySubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "UI/LocalWidgetManager.h"
#include "UI/InitializeUtilityInterface.h"

// Sets default values for this component's properties
UGameStateTimerComponent::UGameStateTimerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	SetIsReplicatedByDefault(true);

	WarningTime = FTimespan::FromSeconds(60.0);
}

void UGameStateTimerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxTime);
	DOREPLIFETIME(ThisClass, RemainTime);
}

// Called when the game starts
void UGameStateTimerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerGameStateBase = Cast<AGameStateBase>(GetOwner());
	InitializeTimer();
	IWidgetInitializableInterface::Execute_InitWidget(this);
}

void UGameStateTimerComponent::InitWidget_Implementation()
{
	// Local Client 확인
	if (IsValid(OwnerGameStateBase) == false || OwnerGameStateBase->IsNetMode(ENetMode::NM_DedicatedServer) == true)
	{
		return;
	}

	// WidgetManager 유효성 확인
	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	// Widget 생성 및 유효성 확인
	UUserWidget* TimerWidget = WidgetManager->AddWidget(FName("Timer"), TimerWidgetClass);
	if (IsValid(TimerWidget) == false)
	{
		return;
	}

	// Widget 초기 설정 진행
	IInitializeUtilityInterface::Execute_InitializeWidgetByComponent(TimerWidget, this);

	/*
	* HUD Widget 구현 후 TimerWidget을 HUD Widget의 Child로 추가하도록 수정 필요
	*/
	UE_LOG(LogTemp, Warning, TEXT("# TimerWidget을 HUD Widget의 Child로 추가하도록 수정 필요"));
	TimerWidget->AddToPlayerScreen();
}

// 제한 시간을 초기화하는 함수
// 현재 남은 시간을 초기값으로 설정
void UGameStateTimerComponent::InitializeTimer()
{
	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	RemainTime = MaxTime;
}

// 타이머를 시작하는 함수
void UGameStateTimerComponent::StartTimer()
{
	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	// World의 Timer Manager 얻기
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// 현재 타이머의 실행 상태 확인 (시간 제한이 적용중인 상태)
	if (TimerManager.IsTimerActive(TickTimer) == true)
	{
		return;
	}

	// 현재 타이머의 일시 정지 상태 확인
	if (TimerManager.IsTimerPaused(TickTimer) == true)
	{
		TimerManager.UnPauseTimer(TickTimer);
		return;
	}

	// 새로운 Timer 설정

	// Delegate 설정
	FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &UGameStateTimerComponent::DecreaseRemainTime, 1.0);

	// 1초마다 DecreaseRemainTime 함수 호출
	GetWorld()->GetTimerManager().SetTimer(TickTimer, Callback, 1, true);
}

// 타이머를 일시 정지하는 함수
void UGameStateTimerComponent::PauseTimer()
{
	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	// 1초마다 남은 시간 1초씩 감소
	GetWorld()->GetTimerManager().PauseTimer(TickTimer);
}

void UGameStateTimerComponent::SetMaxTime(const FTimespan& NewMaxTime)
{
	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	MaxTime = NewMaxTime;
}

void UGameStateTimerComponent::SetRemainTime(const FTimespan& NewRemainTime)
{
	UE_LOG(LogTemp, Warning, TEXT("This Function is for Debug Only"));

	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	RemainTime = NewRemainTime;
}

void UGameStateTimerComponent::DecreaseRemainTime(double DeltaSeconds)
{
	// Server 환경 확인
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	const FTimespan PrevRemainTime = RemainTime;

	// 남은 시간 1초 감소
	RemainTime -= FTimespan::FromSeconds(DeltaSeconds);

	// 시간 종료 임박 경계(WarningTime)를 통과하는 순간에만 1회 알림
	if (PrevRemainTime > WarningTime && RemainTime <= WarningTime)
	{
		if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
		{
			NotifySubsystem->NotifyEvent(RETag::Event::Timer::AlmostOver,
				FString::Printf(TEXT("남은 시간이 %d초 이하입니다"), static_cast<int32>(WarningTime.GetTotalSeconds())));
		}
	}

	// Listen Server 환경을 고려하여 남은 시간 변화 이벤트 실행
	OnRep_RemainTime();
}
