// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

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
class ROOMESCAPE_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
#pragma region Widget Components

	// 포커싱 된 오브젝트의 아이콘을 출력하는 Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_ObjectIcon;

	// 포커싱 된 오브젝트의 이름을 출력하는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBlock_ObjectName;

	// 포커싱 상태에 맞추어 Crosshair를 출력하는 Image
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_Crosshair;

	// InputAction Prompt를 나타내는 영역
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> HorizontalBox_InputActionNavigation;

#pragma endregion

	// Crosshair 이미지 하단에 노출되는 InputAction Prompt의 단위별 Widget 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InputActionPromptWidgetClass;

private:
	UPROPERTY()
	FVector2D PreviousSize = FVector2D::ZeroVector;

public:
	// Crosshair 상단에 노출되는 객체의 아이콘, 이름 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetObjectInfo(UTexture2D* Texture_ObjectIcon, const FText& ObjectName);

	// Crosshair 하단에 노출되는 InputAction Navigation 내용을 설정하는 함수
	UFUNCTION(BlueprintCallable)
	void SetInputActionNavigationBar(const TArray<FInputActionPromptData>& InputActionPrompts);

protected:
	// !!! Not Completed !!! Don't Use
	// Crosshair 이미지가 화면 정중앙에 위치할 수 있도록 Widget의 위치를 조정하는 함수
	void SetCrosshairToCenter(const FVector2D& CurrentSize);
};
