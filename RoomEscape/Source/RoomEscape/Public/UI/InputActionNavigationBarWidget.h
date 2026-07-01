// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputActionNavigationBarWidget.generated.h"

USTRUCT(BlueprintType)
struct ROOMESCAPE_API FInputActionPromptData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Texture_InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayActionName;
};

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UInputActionNavigationBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 여러개의 InputAction Prompt Widget들을 관리하는 HorizontalBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> HorizontalBox_Root;

	// 화면에 InputAction Prompt를 표시할 Widget 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UInputActionPromptWidget> InputActionPromptWidgetClass;

public:
	// NavigationBar에 표시된 InputAction Prompt 목록을 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetInputActionNavigationBar(const TArray<FInputActionPromptData>& InputActionPrompts);
};
