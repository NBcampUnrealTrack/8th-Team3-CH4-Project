// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "GlassMazeManager.generated.h"

class AGlassWall;
class AMazeTrapTile;
class AREPlayerCharacter;

/**
 * 유리벽 미로 매니저.
 * 벽/함정/출구/레버는 레벨 디자이너가 레벨에 직접 배치하고, 각 액터의 Manager 참조를
 * 이 매니저 인스턴스로 지정한다. 매니저는 레버 요청을 받아 LeverID가 일치하는 개폐 벽을
 * 토글하고, 출구 도달/함정 발동을 서버 권한으로 판정할 뿐 배치에는 관여하지 않는다.
 */
UCLASS()
class ROOMESCAPE_API AGlassMazeManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	// 개폐 벽이 BeginPlay에서 자기 자신을 등록
	void RegisterOpenableWall(AGlassWall* Wall);

	// 서버 전용 - 레버가 호출. LeverID가 일치하는 개폐 벽을 모두 bOpen 상태로
	void SetWallsOpenByLever(FName LeverID, bool bOpen);

	// 서버 전용 - 함정 타일이 호출. 연출/알림만 처리 (리셋은 레벨 재시작으로 대체)
	void HandleTrapTriggered(AMazeTrapTile* Trap, AREPlayerCharacter* Victim);

	// 서버 전용 - 출구 볼륨이 호출
	void HandleExitReached(AActor* Reacher);

protected:
	virtual void HandlePuzzleSolved() override;

	UPROPERTY()
	TArray<TObjectPtr<AGlassWall>> OpenableWalls;
};
