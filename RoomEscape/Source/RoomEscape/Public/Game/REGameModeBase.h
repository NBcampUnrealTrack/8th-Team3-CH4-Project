#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "REGameModeBase.generated.h"

UCLASS()
class ROOMESCAPE_API AREGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
};
