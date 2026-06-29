#include "Game/REGameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AActor* AREGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 맵의 모든 PlayerStart 액터를 찾아 배열에 추가
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	
	// 방장이 아직 스폰되지 않았다면 RoomA, 스폰되었다면 RoomB를 목표 태그로 설정
	FName TargetTag = Player->IsLocalController() ? FName("RoomB") : FName("RoomA");
	
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
