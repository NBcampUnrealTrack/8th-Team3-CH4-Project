// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LocalWidgetManagerSettings.generated.h"

/**
 * Unreal Project Settings 창에 설정 값을 추가하여 .ini에 해당 내용을 기록 및 로드하기 위한 클래스
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Local Widget Manager"))
class WIDGETUTILITY_API ULocalWidgetManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Widget")
	TSoftObjectPtr<class UWidgetManagerConfigDataAsset> WidgetManagerConfig;
};
