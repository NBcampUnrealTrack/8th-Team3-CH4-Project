// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InputActionPromptWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "UI/InputActionNavigationBarWidget.h"

void UInputActionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInputActionPromptWidget::InitializeIndicatorPrompt(const FInputActionPromptData& InputActionPrompt)
{
	// TextBlock 유효성 확인
	if (IsValid(TextBlock_InputAction) == true && InputActionPrompt.DisplayActionName.IsEmpty() == false)
	{
		// TextBlock 내용 설정
		TextBlock_InputAction->SetText(InputActionPrompt.DisplayActionName);
	}

	// Image 유효성 확인
	if (IsValid(Image_InputAction) == true && InputActionPrompt.Texture_InputAction.IsNull() == false)
	{
		// 이미지에 사용되는 Texture 설정
		Image_InputAction->SetBrushFromSoftTexture(InputActionPrompt.Texture_InputAction);

		//// Texture 에셋이 기존에 로드되어 있지 않으면 비동기 로드 실행
		//if (InputActionPrompt.Texture_InputAction.IsValid() == false)
		//{
		//	// 비동기 로드를 위한 StreamableManager 참조
		//	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

		//	// 비동기 로드 완료 시 호출 함수 설정
		//	FStreamableDelegate AsyncLoadCallback = FStreamableDelegate::CreateUObject(this, &UInputActionPromptWidget::OnCompleteAsyncLoadInputActionIcon, InputActionPrompt.Texture_InputAction);

		//	// 비동기 로드 시작
		//	AsyncLoadingHandle = StreamableManager.RequestAsyncLoad(InputActionPrompt.Texture_InputAction.ToSoftObjectPath(), AsyncLoadCallback);
		//}
		//// 동기 방식으로 에셋 로드
		//else
		//{
		//	OnCompleteAsyncLoadInputActionIcon(InputActionPrompt.Texture_InputAction);
		//}
	}
}

void UInputActionPromptWidget::OnCompleteAsyncLoadInputActionIcon(TSoftObjectPtr<UTexture2D> Texture_InputAction)
{
	// 에셋이 정상적으로 로드되었는지 확인
	UTexture2D* InputActionIcon = Texture_InputAction.Get();
	if (IsValid(InputActionIcon) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Asset was loaded for InputAction Prompt"));
		return;
	}

	// 이미지에 사용되는 Texture 및 크기 설정
	Image_InputAction->SetBrushFromTexture(InputActionIcon);
	Image_InputAction->SetDesiredSizeOverride({ 32, 32 });
}
