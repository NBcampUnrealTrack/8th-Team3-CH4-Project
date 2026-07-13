// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/REGameModeComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Components/RESessionPlayerStateComponent.h"

// Sets default values for this component's properties
UREGameModeComponent::UREGameModeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UREGameModeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerGamemode = Cast<AGameModeBase>(GetOwner());
}

// Called every frame
void UREGameModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UREGameModeComponent::ConfigureAllocatedMatch(int32 InExpectedPlayerCount, const FString& InTargetMapPath)
{
    // Allocation 정보는 서버에서만 설정해야 합니다.
    if (OwnerGamemode->HasAuthority() == false)
    {
        return;
    }

    // 최소 한 명 이상의 플레이어가 지정되었는지 확인합니다.
    if (InExpectedPlayerCount <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid expected player count: %d"), InExpectedPlayerCount);
        return;
    }

    // 서버에서 실제로 시작할 수 있는 맵만 화이트리스트로 관리합니다.
    static const TSet<FString> AllowedMapPaths =
    {
        // 기본 맵을 허용합니다.
        TEXT("/Game/Level/MainLevel?listen"),
    };

    // Allocation으로 전달된 맵이 서버의 허용 목록에 있는지 확인합니다.
    if (MapPaths.Contains(InTargetMapPath) == false)
    {
        // 거부한 맵 경로를 서버 로그에 기록합니다.
        UE_LOG(LogTemp, Error, TEXT("Rejected target map: %s"), *InTargetMapPath);
        return;
    }

    // 해당 매치에 접속해야 하는 총 플레이어 수를 저장합니다.
    ExpectedPlayerCount = InExpectedPlayerCount;

    // 모든 플레이어가 준비되었을 때 시작할 맵을 저장합니다.
    TargetMapPath = InTargetMapPath;

    // 적용된 Allocation 정보를 서버 로그에 기록합니다.
    UE_LOG(LogTemp, Log, TEXT("Allocated match configured: Players=%d, Map=%s"), ExpectedPlayerCount, *TargetMapPath);

    // Allocation 수신 전에 플레이어가 준비되었을 가능성을 고려해 즉시 재검사합니다.
    TryStartTargetMap();
}

void UREGameModeComponent::MarkPlayerReady(APlayerController* PlayerController)
{
    // Ready 상태 변경은 서버에서만 처리합니다.
    if (OwnerGamemode->HasAuthority() == false)
    {
        // 클라이언트에서 직접 호출된 요청은 무시합니다.
        return;
    }

    // 유효하지 않은 PlayerController가 전달되었는지 확인합니다.
    if (IsValid(PlayerController) == false)
    {
        return;
    }

    //// 해당 플레이어의 Ready 상태용 PlayerState를 가져옵니다.
    //AReadyPlayerState* ReadyPlayerState = PlayerController->GetPlayerState<AReadyPlayerState>();

    //// PlayerState가 아직 준비되지 않았는지 확인합니다.
    //if (ReadyPlayerState == nullptr)

    //    // PlayerState가 없는 경우의 처리를 시작합니다.
    //{

    //    // Ready 상태를 저장할 대상이 없으므로 종료합니다.
    //    return;

    //    // PlayerState가 없는 경우의 처리를 종료합니다.
    //}

    //// 서버 권한으로 해당 플레이어를 준비 완료 상태로 설정합니다.
    //ReadyPlayerState->SetReady(true);

    //// Ready 처리가 완료된 플레이어를 서버 로그에 기록합니다.
    //UE_LOG(
    //    LogTemp,
    //    Log,
    //    TEXT("Player ready: %s"),
    //    *ReadyPlayerState->GetPlayerName()
    //);

    // 상태 변경 후 전체 게임 시작 조건을 다시 검사합니다.
    TryStartTargetMap();
}

void UREGameModeComponent::TryStartTargetMap()
{
    // 게임 시작 판정은 서버에서만 수행합니다.
    if (OwnerGamemode->HasAuthority() == false)
    {
        return;
    }

    // 이미 ServerTravel을 시작했는지 확인합니다.
    if (bTravelStarted)
    {
        return;
    }

    // 유효한 Allocation 인원이 설정되었는지 확인합니다.
    if (ExpectedPlayerCount <= 0)
    {
        return;
    }

    // 이동할 맵이 설정되었는지 확인합니다.
    if (TargetMapPath.IsEmpty())
    {
        return;
    }

    // 현재 서버 월드를 가져옵니다.
    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        return;
    }

    // 현재 접속한 플레이어 목록을 관리하는 GameState를 가져옵니다.
    AGameStateBase* CurrentGameState = World->GetGameState();
    // GameState가 아직 생성되지 않았는지 확인합니다.
    if (IsValid(CurrentGameState) == false)
    {
        return;
    }

    // 현재 접속 인원이 Allocation의 예상 인원과 일치하는지 확인합니다.
    if (CurrentGameState->PlayerArray.Num() != ExpectedPlayerCount)
    {
        return;
    }

    // 현재 접속한 모든 플레이어의 상태를 순회합니다.
    for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
    {
        // 예상한 PlayerState 타입인지 확인합니다.
        if (IsValid(PlayerState) == false)
        {
            return;
        }

        // 플레이어가 Ready 상태인지 확인
        URESessionPlayerStateComponent* SessionComponent = PlayerState->FindComponentByClass<URESessionPlayerStateComponent>();
        if (IsValid(SessionComponent) == false || SessionComponent->IsPlayerReady() == false)
        {
            return;
        }
    }

    // 이후에 들어오는 중복 Ready 요청이 추가 이동을 발생시키지 않도록 잠급니다.
    bTravelStarted = true;

    // 모든 시작 조건이 충족되었음을 서버 로그에 기록합니다.
    UE_LOG(LogTemp, Log, TEXT("All players ready. Starting map: %s"), *TargetMapPath);

    // 서버와 연결된 모든 클라이언트를 대상 맵으로 이동시킵니다.
    bool bTravelAccepted = World->ServerTravel(TargetMapPath, false, false);

    // ServerTravel 요청이 정상적으로 접수되었는지 확인합니다.
    if (bTravelAccepted == false)
    {
        // 운영 정책에 따라 다시 시도할 수 있도록 잠금 상태를 해제합니다.
        bTravelStarted = false;

        // 실패한 대상 맵을 서버 로그에 기록합니다.
        UE_LOG(LogTemp, Error, TEXT("ServerTravel failed: %s"), *TargetMapPath);
    }

}

