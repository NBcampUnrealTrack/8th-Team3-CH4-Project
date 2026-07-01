// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputActionPromptWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UInputActionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
#pragma region Widget Components

	// 입력할 Key 또는 Button의 Icon을 나타내는 Image
	// Ex) Ⓨ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_InputAction;

	// 해당 Key 또는 Button의 입력에 대한 동작을 설명하는 TextBlock
	// Ex) 잡기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBlock_InputAction;

#pragma endregion
	
private:
	// 비동기 에셋 로드를 위한 Handle
	TSharedPtr<struct FStreamableHandle> AsyncLoadingHandle;

public:
	// InputAction Indicator Widget의 이미지와 텍스트를 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void InitializeIndicatorPrompt(const struct FInputActionPromptData& InputActionPrompt);

protected:
	UFUNCTION()
	void OnCompleteAsyncLoadInputActionIcon(TSoftObjectPtr<UTexture2D> Texture_InputAction);
};
