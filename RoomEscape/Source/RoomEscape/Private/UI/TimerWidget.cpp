// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TimerWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/GameStateTimerComponent.h"

void UTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTimerWidget::InitializeWidgetByContextObject_Implementation(UObject* ContextObject)
{
}

void UTimerWidget::InitializeWidgetByComponent_Implementation(UActorComponent* Component)
{
	UGameStateTimerComponent* TimerComponent = Cast<UGameStateTimerComponent>(Component);
	if (IsValid(TimerComponent) == false)
	{
		return;
	}
	
	// 현재 시간 제한에 맞추어 UI 초기 설정 진행
	OnTimerValueChanged(TimerComponent->GetRemainTime(), TimerComponent->GetMaxTime());

	// 이벤트 연결
	TimerComponent->OnTimerValueChanged.AddDynamic(this, &UTimerWidget::OnTimerValueChanged);
}

void UTimerWidget::InitializeWidgetByActor_Implementation(AActor* Actor)
{
}

void UTimerWidget::OnTimerValueChanged(const FTimespan& RemainingTime, const FTimespan& MaxTime)
{
	// TextBlock 유효성 확인
	if (IsValid(TextBlock_Time) == true)
	{
		// TextBlock의 Text 업데이트
		FString TimerString = FString::Printf(TEXT("%02d : %02d"), RemainingTime.GetMinutes(), RemainingTime.GetSeconds());
		TextBlock_Time->SetText(FText::FromString(TimerString));

		// 남은 시간에 따라 TextBlock 색상 변경
		double RemainingMinutes = RemainingTime.GetTotalMinutes();
		FColor TimerColor = FColor::Black;
		if (RemainingMinutes < 1)
		{
			TimerColor = FColor::Red;
		}
		if (RemainingMinutes < 5)
		{
			TimerColor = FColor::Orange;
		}
		TextBlock_Time->SetColorAndOpacity(FSlateColor(TimerColor));
	}

	// 남은 시간 비율 계산
	double factor = FTimespan::Ratio(RemainingTime, MaxTime);

	// ProgressBar 비율 조정
	if (IsValid(ProgressBar_Left) == true && IsValid(ProgressBar_Right) == true)
	{
		ProgressBar_Left->SetPercent(factor);
		ProgressBar_Right->SetPercent(factor);
	}
}
