// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "REWidgetManager.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;

// Widget의 클래스와 해당 Widget이 추가될 Layer의 Tag를 담는 구조체
USTRUCT(BlueprintType)
struct ROOMESCAPE_API FREWidgetLayerData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag LayerTag;
};

// 사용되는 Layer[UCommonActivatableWidgetContainerBase*] 추적 관리용 배열의 Wraper 구조체
USTRUCT(BlueprintType)
struct ROOMESCAPE_API FLayerStack
{
	GENERATED_BODY()

public:
	// 현재 사용중인 Layer[UCommonActivatableWidgetContainerBase*]를 관리하기 위한 배열
	UPROPERTY()
	TArray<TWeakObjectPtr<UCommonActivatableWidgetContainerBase>> LayerStack;
};

/**
 * Not Completed
 * Test WidgetManager Using GameplayTags
 */
UCLASS()
class ROOMESCAPE_API UREWidgetManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> RootWidgetInstance;

	// FGameplayTag 별로 추적되는 현재 레이어의 위치를 관리하는 Map
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FLayerStack> LayerMap;

public:
	// LayerTag로 등록되어있는 Layer[UCommonActivatableWidgetContainerBase*] 목록에서 현재 유효한 객체를 반환하는 함수
	// 현재 유효한 Layer가 존재하지 않으면 nullptr 반환
	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidgetContainerBase* GetWidgetLayer(FGameplayTag LayerTag);

#pragma region Layer[UCommonActivatableWidgetContainerBase*]

	// Layer[UCommonActivatableWidgetContainerBase*]를 LayerTag로 등록 또는 업데이트
	UFUNCTION(BlueprintCallable)
	bool PushWidgetLayer(UCommonActivatableWidgetContainerBase* Layer, FGameplayTag LayerTag);

	// 현재 LayerTag로 등록되어있는 최상단의 Layer[UCommonActivatableWidgetContainerBase*]를 제거하는 함수
	UFUNCTION(BlueprintCallable)
	bool PopWidgetLayer(FGameplayTag LayerTag);

	// 현재 LayerTag로 등록되어있는 Layer[UCommonActivatableWidgetContainerBase*] 전체를 제거하는 함수
	UFUNCTION(BlueprintCallable)
	bool ClearWidgetLayer(FGameplayTag LayerTag);

#pragma endregion

	// RootLayout을 담당하는 Widget을 생성, 설정 및 표시
	UFUNCTION(BlueprintCallable)
	void SetRootLayout(TSubclassOf<UUserWidget> RootWidgetClass);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UUserWidget* GetRootWidget() const { return RootWidgetInstance; }

#pragma region Widget In Layer

	UCommonActivatableWidget* AddWiget(const FREWidgetLayerData& WidgetLayerData);

#pragma endregion

protected:
	void MoveWidgetsInLayer(UCommonActivatableWidgetContainerBase* PreviousLayer, UCommonActivatableWidgetContainerBase* CurrentLayer);
};
