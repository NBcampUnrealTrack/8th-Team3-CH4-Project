// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Progression/REProgressionTypes.h"
#include "REProgressionManager.generated.h"

class AREPuzzleManager;
class AREBombDefusalManager;
class AREProgressionDoor;
class AREPuzzleResetPoint;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREProgressionStageChangedSignature, EREGameStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREProgressionSimpleSignature);

/**
 * 게임 진행 매니저 (레벨에 1개 배치).
 * 방 퍼즐 3종 클리어 → 지하 문 개방(EscapePhase), 폭탄 해체 → B방 문 개방(Escaped/엔딩),
 * 폭탄 실패 → 배드엔딩 연출 이벤트 → 2부(탈출 시퀀스) 재시작 (방 퍼즐 클리어 상태 유지).
 * 판정/전환은 전부 서버 권한. UI는 OnStageChanged / OnBadEndingStarted / OnEscapePhaseRestarted 구독(양쪽 발화).
 */
UCLASS(Blueprintable)
class ROOMESCAPE_API AREProgressionManager : public AActor
{
	GENERATED_BODY()

public:
	AREProgressionManager();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FREProgressionStageChangedSignature OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FREProgressionSimpleSignature OnBadEndingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FREProgressionSimpleSignature OnEscapePhaseRestarted;

protected:
	// --- 레벨 배선 (배치 인스턴스에서 지정)
	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TArray<TObjectPtr<AREPuzzleManager>> RoomPuzzles;

	// 배드엔딩 시 리셋·재활성화되는 탈출 퍼즐(미로/타일패스). 폭탄 매니저는 자동 포함되니 넣지 말 것.
	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TArray<TObjectPtr<AREPuzzleManager>> EscapePuzzles;

	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TObjectPtr<AREBombDefusalManager> BombManager;

	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TArray<TObjectPtr<AREProgressionDoor>> BasementDoors;

	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TArray<TObjectPtr<AREProgressionDoor>> EscapeDoors;

	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TObjectPtr<AREPuzzleResetPoint> RestartPointA;

	UPROPERTY(EditInstanceOnly, Category = "Progression|Wiring")
	TObjectPtr<AREPuzzleResetPoint> RestartPointB;

	// 배드엔딩(사망 연출) 후 2부 재시작까지의 기본 대기 시간.
	// 실제 대기 시간은 BombManager의 실패 메시지 최소 표시 시간보다 짧아질 수 없습니다.
	UPROPERTY(EditAnywhere, Category = "Progression|Rule", meta = (ClampMin = "0.0", Units = "s"))
	float BadEndingDelaySeconds = 5.0f;

	// 리슨 서버 호스트를 Player A(지하 진입자)로 볼지 여부
	UPROPERTY(EditAnywhere, Category = "Progression|Rule")
	bool bHostIsPlayerA = true;

	// 테스트용 시작 스테이지. 폭탄 테스트 레벨처럼 방 퍼즐 없이 2부부터 검증할 때 EscapePhase로 지정
	UPROPERTY(EditInstanceOnly, Category = "Progression|Rule")
	EREGameStage StartStage = EREGameStage::RoomPhase;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStage, VisibleInstanceOnly, BlueprintReadOnly, Category = "Progression|Runtime", meta = (AllowPrivateAccess = "true"))
	EREGameStage CurrentStage = EREGameStage::RoomPhase;

	FTimerHandle BadEndingTimerHandle;

public:
	UFUNCTION(BlueprintPure, Category = "Progression")
	EREGameStage GetCurrentStage() const;

protected:
	UFUNCTION()
	void HandleRoomPuzzleSolved();

	UFUNCTION()
	void HandleBombSolved();

	UFUNCTION()
	void HandleBombFailed();

	UFUNCTION()
	void OnRep_CurrentStage();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBadEndingStarted();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEscapePhaseRestarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression")
	void ReceiveStageChanged(EREGameStage NewStage);

private:
	void SetStage(EREGameStage NewStage);
	bool AreAllRoomPuzzlesSolved() const;
	void EnterEscapePhase();
	void RestartEscapePhase();
	void ResetAndActivateEscapePuzzles();
	void TeleportPlayersToRestartPoints();
	void SetDoorsOpen(const TArray<TObjectPtr<AREProgressionDoor>>& Doors, bool bOpen);
	TArray<AREPuzzleManager*> GetEscapePuzzlesIncludingBomb() const;
	void NotifyProgress(const FGameplayTag& EventTag, const FString& Message) const;
};
