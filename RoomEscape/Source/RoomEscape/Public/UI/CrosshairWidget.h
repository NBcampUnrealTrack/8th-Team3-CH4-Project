// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	// 포커싱 상태에 맞추어 Crosshair를 출력하는 Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_Crosshair;

	// 기본 Crosshair Texture
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> Texture_DefaultCrosshair;

public:
	// Crosshair에 사용되는 이미지를 기본값으로 초기화하는 함수
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void InitializeCrosshairImageTexture() { SetCrosshairImageTexture(Texture_DefaultCrosshair); }

	// Crosshair를 표시하는 Texture를 변경하는 함수
	UFUNCTION(BlueprintCallable)
	void SetCrosshairImageTexture(TSoftObjectPtr<UTexture2D> Texture_Crosshair);

protected:
	// Crosshair 이미지가 화면 정중앙에 위치할 수 있도록 Widget의 위치를 조정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetCrosshairToCenter();
};
