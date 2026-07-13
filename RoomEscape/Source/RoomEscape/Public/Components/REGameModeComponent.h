// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "REGameModeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOMESCAPE_API UREGameModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UREGameModeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
    TObjectPtr<AGameModeBase> OwnerGamemode;

    // 사용되는 Map 목록
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FString> MapPaths;

    // Allocation에서 전달받은 매치 참가 예정 인원입니다.
    int32 ExpectedPlayerCount;

    // 모든 플레이어가 준비되었을 때 이동할 맵 경로입니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetMapPath = TEXT("");

    // ServerTravel이 여러 번 호출되는 것을 방지합니다.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
    bool bTravelStarted;

public:
    // Multiplay Allocation 결과를 GameMode에 적용할 수 있도록 노출합니다.
    UFUNCTION(BlueprintCallable, Category = "Multiplay")
    // 매치의 예상 인원과 시작할 맵을 설정합니다.
    void ConfigureAllocatedMatch(int32 InExpectedPlayerCount, const FString& InTargetMapPath);

    // 클라이언트의 Ready 요청을 서버에서 처리할 수 있도록 노출합니다.
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    // 지정된 플레이어를 Ready 상태로 변경합니다.
    void MarkPlayerReady(APlayerController* PlayerController);

protected:
    // 모든 시작 조건을 검사하고 조건이 충족되면 맵을 전환합니다.
    void TryStartTargetMap();

};
