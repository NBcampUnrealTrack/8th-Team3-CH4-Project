// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UObjectInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 객체의 아이콘을 표시하는 Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_ObjectIcon;

	// 객체의 이름을 표시하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBlock_ObjectName;

public:
	// UI에 표시되는 객체의 아이콘 및 이름을 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetVisualObjectInfo(const TSoftObjectPtr<UTexture2D> Texture_ObjectIcon, const FText& ObjectName);
};
