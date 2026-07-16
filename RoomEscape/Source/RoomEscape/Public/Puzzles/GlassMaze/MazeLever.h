// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "MazeLever.generated.h"

class UStaticMeshComponent;

/**
 * 1층 관찰자(A)용 레버. 상호작용 시 매니저에 같은 LeverID의 개폐 벽 토글을 요청.
 * 레버는 벽의 실제 상태를 모른다 - 요청만 한다 (정답은 매니저만).
 * 매니저 참조/Active 검사/서버 실행은 AREPuzzleInteractableActor가 처리.
 */
UCLASS()
class ROOMESCAPE_API AMazeLever : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AMazeLever();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버 전용 - 미로 리셋 시 매니저가 호출. 시각 상태만 Off로 (벽은 매니저가 직접 닫음)
	void ResetLeverState();

protected:
	virtual void BeginPlay() override;

	// 서버에서 실행 (베이스의 ProcessServerInteract가 권한/Active 검증 후 호출)
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION()
	void OnRep_IsOn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Maze", meta = (DisplayName = "On Lever Toggled"))
	void BP_OnLeverToggled(bool bNewOn);

	UPROPERTY(EditAnywhere, Category = "Maze")
	FName LeverID = NAME_None;

	// 레버 시각 상태 (On = 벽 열림 요청 상태)
	UPROPERTY(ReplicatedUsing = OnRep_IsOn)
	bool bIsOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	TObjectPtr<UStaticMeshComponent> LeverMesh;
};
