// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InitializeUtilityInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInitializeUtilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WIDGETUTILITY_API IInitializeUtilityInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 임의 컴포넌트에 맞추어 Widget의 초기 설정을 진행하는 함수
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWidgetByContextObject(UObject* ContextObject);

	// 임의 컴포넌트에 맞추어 Widget의 초기 설정을 진행하는 함수
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWidgetByComponent(UActorComponent* Component);

	// 임의 컴포넌트에 맞추어 Widget의 초기 설정을 진행하는 함수
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWidgetByActor(AActor* Actor);

protected:
	virtual void InitializeWidgetByContextObject_Implementation(UObject* ContextObject) = 0;

	virtual void InitializeWidgetByComponent_Implementation(UActorComponent* Component) = 0;

	virtual void InitializeWidgetByActor_Implementation(AActor* Actor) = 0;
};
