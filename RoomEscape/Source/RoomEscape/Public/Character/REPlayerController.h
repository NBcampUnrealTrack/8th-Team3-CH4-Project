#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "REPlayerController.generated.h"

class UInputMappingContext;
class AREBombDefusalManager;

UCLASS()
class ROOMESCAPE_API AREPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Debug 전용 : 알림 파이프라인 테스트용 콘솔 명령 (사용 예 : RETestNotify Hello)
	UFUNCTION(Exec)
	void RETestNotify(const FString& Message);

	/** WBP_BombDefusal에서 선택한 선을 서버의 Bomb Actor로 전달합니다. */
	UFUNCTION(Server, Reliable, Category = "Bomb Defusal")
	void ServerSubmitBombWireSelection(AREBombDefusalManager* BombManager, int32 WireIndex);

	/** WBP_BombDefusal에서 선택한 버튼을 서버의 Bomb Actor로 전달합니다. */
	UFUNCTION(Server, Reliable, Category = "Bomb Defusal")
	void ServerSubmitBombButtonSelection(AREBombDefusalManager* BombManager, FName ButtonId);

	/** 서버에서 상호작용을 승인한 뒤, 이 컨트롤러를 소유한 클라이언트에 폭탄 WBP를 엽니다. */
	UFUNCTION(Client, Reliable, Category = "Bomb Defusal")
	void ClientOpenBombDefusalWidget(AREBombDefusalManager* BombManager);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	int32 MappingPriority;

};
