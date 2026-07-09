// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LocalWidgetManager.h"

ULocalWidgetManager* ULocalWidgetManager::GetInstance(const UObject* WorldContextObject)
{
	// 객체 유효성 확인
	if (IsValid(WorldContextObject) == false)
	{
		//UE_LOG(LogTemp, Warning, TEXT("# %s - Invalid Context Object"), FUNCTION_SIG);
		return nullptr;
	}

	// 월드 유효성 확인
	UWorld* World = WorldContextObject->GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	// LocalPlayer의 Controller 객체 얻기 및 유효성 확인
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld());
	if (IsValid(LocalPlayerController) == false)
	{
		return nullptr;
	}

	// LocalPlayer 얻기
	ULocalPlayer* LocalPlayer = LocalPlayerController->GetLocalPlayer();

	// SubSystem 얻기
	return IsValid(LocalPlayer) == true ? LocalPlayer->GetSubsystem<ULocalWidgetManager>() : nullptr;
}

UWidget* ULocalWidgetManager::FindWidget(FName WidgetName)
{
	// 찾는 Widget의 이름 유효성 확인
	if (WidgetName.IsNone() == true)
	{
		return nullptr;
	}

	// Key를 통해 WidgetInstance 찾기
	UWidget* WidgetInstance = WidgetMap.FindRef(WidgetName);
	return WidgetInstance;
}

UUserWidget* ULocalWidgetManager::AddWidget(FName WidgetName, TSubclassOf<UUserWidget> WidgetClass)
{
	// 추가하려는 Widget의 이름 및 Class 유효성 확인
	if (WidgetName.IsNone() == true || IsValid(WidgetClass) == false)
	{
		return nullptr;
	}

	// 동일한 키로 저장된 Widget Instance 얻기
	UWidget* WidgetInstance = FindWidget(WidgetName);
	if (IsValid(WidgetInstance) == true)
	{
		// 생성하려는 클래스로부터 파생된 Widget 여부 확인
		if (WidgetInstance->IsA(WidgetClass) == true)
		{
			// 기존 Widget Instance 반환
			return Cast<UUserWidget>(WidgetInstance);
		}

		// Widget 생성 및 등록 실패
		return nullptr;
	}
	// 새로운 Widget Instance 생성 및 저장

	// 기존 Widget Instance 삭제
	WidgetMap.Remove(WidgetName);

	// 로컬 플레이어 객체 얻기
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (IsValid(LocalPlayer) == false)
	{
		return nullptr;
	}

	// Instance 생성
	UUserWidget* UserWidgetInstance = CreateWidget<UUserWidget>(LocalPlayer->PlayerController, WidgetClass);

	// Instance 저장
	WidgetMap.Add(WidgetName, UserWidgetInstance);

	// 해당 이름의 Widget을 기다리고 있는 이벤트 실행
	if (PendingTasks.Contains(WidgetName) == true)
	{
		for (const FOnWidgetCreatedDelegate& Callback : PendingTasks[WidgetName])
		{
			Callback.ExecuteIfBound(UserWidgetInstance);
		}
		PendingTasks.Remove(WidgetName);
	}

	// Instance 반환
	return UserWidgetInstance;
}

bool ULocalWidgetManager::AddWidgetInstance(FName WidgetName, UWidget* WidgetInstance)
{
	// 추가하려는 Widget의 이름 및 Class 유효성 확인
	if (WidgetName.IsNone() == true || IsValid(WidgetInstance) == false)
	{
		//UE_LOG(LogTemp, Warning, TEXT("# %s - Invalid Widget Name or Instance"), FUNCTION_SIG);
		return false;
	}

	// 동일한 키로 저장된 Widget Instance 얻기
	UWidget* ExistedInstance = FindWidget(WidgetName);
	if (IsValid(ExistedInstance) == true)
	{
		// 저장하려는 Widget Instance와 동일한 Instance 여부 확인
		return ExistedInstance == WidgetInstance;
	}

	// Instance 저장
	WidgetMap.Add(WidgetName, WidgetInstance);

	// 해당 이름의 Widget을 기다리고 있는 이벤트 실행
	if (PendingTasks.Contains(WidgetName) == true)
	{
		for (const FOnWidgetCreatedDelegate& Callback : PendingTasks[WidgetName])
		{
			Callback.ExecuteIfBound(WidgetInstance);
		}
		PendingTasks.Remove(WidgetName);
	}

	return true;
}

void ULocalWidgetManager::RequestAsync(FName WidgetName, const FOnWidgetCreatedDelegate& Callback)
{
	// 조건이 되는 Widget의 이름 유효성 확인
	if (WidgetName.IsNone() == true)
	{
		//UE_LOG(LogTemp, Display, TEXT("# %s - Invalid WidgetName"), FUNCTION_SIG);
		return;
	}

	// 동일한 이름으로 생성되어있는 Widget 검색
	if (UWidget* WidgetInstance = FindWidget(WidgetName))
	{
		// 즉시 실행
		Callback.ExecuteIfBound(WidgetInstance);
		return;
	}

	// 대기열 추가
	PendingTasks.FindOrAdd(WidgetName).Add(Callback);
}

bool ULocalWidgetManager::RemoveWidget(FName WidgetName)
{
	// 제거하려는 Widget의 이름 유효성 확인
	if (WidgetName.IsNone() == true)
	{
		//UE_LOG(LogTemp, Display, TEXT("# %s - Invalid WidgetName"), FUNCTION_SIG);
		return false;
	}

	// 동일한 Key로 등록된 Widget Instance 얻기
	UWidget* WidgetInstance = WidgetMap.FindAndRemoveChecked(WidgetName);
	if (IsValid(WidgetInstance) == true)
	{
		// Instance 삭제
		WidgetInstance->RemoveFromParent();
	}
	return true;
}

void ULocalWidgetManager::SetWidgetHiddenInGame(const FName& WidgetName, bool bNewHidden)
{
	// Widget의 이름 유효성 확인
	if (WidgetName.IsNone() == true)
	{
		//UE_LOG(LogTemp, Display, TEXT("# %s - Invalid WidgetName"), FUNCTION_SIG);
		return;
	}

	// 동일한 Key로 등록된 Widget Instance 얻기
	UWidget* WidgetInstance = FindWidget(WidgetName);
	if (IsValid(WidgetInstance) == true)
	{
		// Visibility 변경
		WidgetInstance->SetVisibility(bNewHidden == true ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	return;
}

void ULocalWidgetManager::ClearWidgetInGame()
{
	// 저장된 Widget Instance 제거
	for (const auto& pair : WidgetMap)
	{
		pair.Value->RemoveFromParent();
	}
	
	// 저장된 Widget 목록 초기화
	WidgetMap.Empty();
	return;
}
