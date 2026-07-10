// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WidgetManagerConfigDataAsset.generated.h"

/**
 * LocalWidgetManger 실행 시 내부 변수에 대하여 config 파일에 저장된 값을 사용하기 위한 데이터 에셋 클래스
 */
UCLASS()
class WIDGETUTILITY_API UWidgetManagerConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // Root UI로 등록될 이름
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root Widget")
    FName RootWidgetName = TEXT("RootWidget");

    // Root UI로 등록될 Widget의 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root Widget")
    TSoftClassPtr<UUserWidget> RootWidgetClass;
};
