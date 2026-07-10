#pragma once

#include "CoreMinimal.h"
#include "REProgressionTypes.generated.h"

UENUM(BlueprintType)
enum class EREGameStage : uint8
{
	RoomPhase,      // 1부: 방 퍼즐 3종
	EscapePhase,    // 2부: 지하 탈출 (미로 → 타일패스 → 폭탄)
	Escaped         // 폭탄 해체 완료 (엔딩)
};
