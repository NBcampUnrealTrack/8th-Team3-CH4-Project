// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WidgetInitializableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWidgetInitializableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 임의 ActorComponent에서 상속받는 인터페이스
 * void Execute_InitWidget 함수를 통해 임의 ActorComponent에서 관리하는 Widget의 초기 설정(생성, 위치 조정 등)을 진행
 */
class WIDGETUTILITY_API IWidgetInitializableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 컴포넌트에서 관리해야 하는 Widget의 초기 설정(생성, 위치 설정 등)을 진행하는 함수
	UFUNCTION(BlueprintNativeEvent)
	void InitWidget();
	virtual void InitWidget_Implementation() = 0;
};
