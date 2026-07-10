// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "LocalWidgetManager.generated.h"

// Widget 생성 또는 등록에 관한 비동기 함수 실행 시 사용되는 함수 대리자
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWidgetCreatedDelegate, UWidget*, CreatedWidget);

/* 
 * GetInstance를 통하여 Subsystem 인스턴스를 획득
 * Find, Add, Remove를 통하여 Widget을 검색, 추가 및 제거
 */
UCLASS()
class WIDGETUTILITY_API ULocalWidgetManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
protected:
	// Root UI의 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName RootWidgetName;

	// Root UI 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> RootWidgetClass;

	// 등록된 Widget Instance들을 이름에 따라 저장하는 컨테이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, TObjectPtr<UWidget>> WidgetMap;

	// 동일한 이름으로 Widget Instance가 등록되었을 때 실행되는 함수들을 저장하는 컨테이너
	TMap<FName, TArray<FOnWidgetCreatedDelegate>> PendingTasks;

public:
	UFUNCTION()
	static ULocalWidgetManager* GetInstance(const UObject* WorldContextObject);

	// 임의 이름으로 등록된 Widget Instance를 반환하는 함수
	UFUNCTION(BlueprintCallable)
	UWidget* FindWidget(FName WidgetName);

	// 임의 이름으로 등록된 Widget Instance를 반환하는 함수
	template<typename TWidget = UWidget>
	TWidget* FindWidget(FName WidgetName)
	{
		static_assert(TIsDerivedFrom<TWidget, UWidget>::Value, "TWidget must be class derived from UWidget");

		UWidget* WidgetInstance = FindWidget(WidgetName);
		if (IsValid(WidgetInstance) == true)
		{
			return Cast<TWidget>(WidgetInstance);
		}
		return nullptr;
	}

	// 임의 클래스로 Widget Instance를 생성, 입력받은 이름으로 등록하는 함수
	// 동일한 이름으로 등록된 WidgetInstance가 해당 클래스에서 파생된 경우 기존 WidgetInstance 반환
	// Instance 생성 및 등록에 실패할 경우 nullptr 반환
	UFUNCTION(BlueprintCallable)
	UUserWidget* AddWidget(FName WidgetName, TSubclassOf<UUserWidget> WidgetClass);

	// 임의 클래스로 Widget Instance를 생성, 입력받은 이름으로 등록하는 함수
	// 동일한 이름으로 등록된 WidgetInstance가 해당 클래스에서 파생된 경우 기존 WidgetInstance 반환
	// Instance 생성 및 등록에 실패할 경우 nullptr 반환
	template<typename TUserWidget = UUserWidget>
	TUserWidget* AddWidget(FName WidgetName, TSubclassOf<UUserWidget> WidgetClass = TUserWidget::StaticClass())
	{
		static_assert(TIsDerivedFrom<TUserWidget, UUserWidget>::Value, "TWidget must be class derived from UUserWidget");

		if (IsValid(WidgetClass) == false || WidgetClass->IsChildOf(TUserWidget::StaticClass()) == false)
		{
			return nullptr;
		}

		UUserWidget* WidgetInstance = AddWidget(WidgetName, WidgetClass);
		if (IsValid(WidgetInstance) == true)
		{
			return Cast<TUserWidget>(WidgetInstance);
		}
		return nullptr;
	}

	// Widget Instance를 임의 이름으로 등록하는 함수
	// Instance 등록에 실패하는 경우 false 반환
	UFUNCTION(BlueprintCallable)
	bool AddWidgetInstance(FName WidgetName, UWidget* WidgetInstance);

	// 임의 이름으로 Widget Instance가 등록되면 동작시키는 비동기 함수
	UFUNCTION(BlueprintCallable)
	void RequestAsync(FName WidgetName, const FOnWidgetCreatedDelegate& Callback);

	// 임의 이름으로 등록된 Widget Instance를 제거하는 함수
	UFUNCTION(BlueprintCallable)
	bool RemoveWidget(FName WidgetName);

	// 임의 이름으로 등록된 Widget Instance의 Visibility를 변경하는 함수
	UFUNCTION(BlueprintCallable)
	void SetWidgetHiddenInGame(const FName& WidgetName, bool bNewHidden);

	// WidgetManager에 등록된 모든 Widget Instance를 제거하는 함수
	UFUNCTION(BlueprintCallable)
	void ClearWidgetInGame();

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetRootWidget();

protected:
	void LoadConfigDataAsset();
};
