#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "REPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class ROOMESCAPE_API AREPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Debug 전용 : 알림 파이프라인 테스트용 콘솔 명령 (사용 예 : RETestNotify Hello)
	UFUNCTION(Exec)
	void RETestNotify(const FString& Message);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	int32 MappingPriority;

};
