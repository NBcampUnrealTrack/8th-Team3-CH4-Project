#include "Game/REGameModeBase.h"
#include "Game/REGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AActor* AREGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 맵의 모든 PlayerStart 액터를 찾아 배열에 추가
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	
	// 로비에서 선택한 호스트 역할 기준으로 스폰 방 결정 (로비 미경유 시 호스트=A)
	bool bHostIsPlayerA = true;
	if (const UREGameInstance* GameInstance = GetGameInstance<UREGameInstance>())
	{
		bHostIsPlayerA = GameInstance->IsHostPlayerA();
	}
	const bool bIsPlayerA = (Player->IsLocalController() == bHostIsPlayerA);
	FName TargetTag = bIsPlayerA ? FName("RoomA") : FName("RoomB");
	
	// 목표 태그와 일치하는 PlayerStart를 찾아 반환
	for (AActor* StartActor : PlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(StartActor);
		
		if (PlayerStart && PlayerStart->PlayerStartTag == TargetTag)
		{
			// 찾은 위치에서 스폰
			return StartActor;
		}
	}
	
	// 태그를 찾지 못했다면 엔진의 기본 스폰 로직을 따라감
	return Super::ChoosePlayerStart_Implementation(Player);
}
