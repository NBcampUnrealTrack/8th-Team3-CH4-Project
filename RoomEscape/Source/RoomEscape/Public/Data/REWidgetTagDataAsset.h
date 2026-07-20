// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "REWidgetTagDataAsset.generated.h"

class UCommonActivatableWidget;

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREWidgetTagDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// <summary>
	/// 검색된 Widget Blueprint의 클래스를 Key로, 해당 Widget Blueprint에 적용할 <seealso cref="FGameplayTag"/>를 저장하는 Map
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Blueprints")
	TMap<TSoftClassPtr<UCommonActivatableWidget>, FGameplayTag> WidgetTagMap;
};
