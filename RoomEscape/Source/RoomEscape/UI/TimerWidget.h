// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InitializeUtilityInterface.h"
#include "TimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UTimerWidget : public UUserWidget, public IInitializeUtilityInterface
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	// IInitializeUtilityInterface을(를) 통해 상속됨
	void InitializeWidgetByContextObject_Implementation(UObject* ContextObject) override;
	void InitializeWidgetByComponent_Implementation(UActorComponent* Component) override;
	void InitializeWidgetByActor_Implementation(AActor* Actor) override;

protected:
#pragma region Widget Components

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBlock_Time;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Right;

#pragma endregion

protected:
	UFUNCTION()
	void OnTimerValueChanged(const FTimespan& RemainingTime, const FTimespan& MaxTime);
};
