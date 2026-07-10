#pragma once

#include "CoreMinimal.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "REBombDefusalManager.generated.h"

class UREBombPatternData;
class AREBombDevice;
class AREBombWire;
class AREBombButton;
class UREBombFeedbackWidget;
class UREFadeWidget;
class AREPuzzleResetPoint;
class APlayerController;
class APlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombTimeChangedSignature, float, RemainingTimeSeconds, float, TimeLimitSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombStepChangedSignature, int32, CurrentStepIndex, int32, TotalStepCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FREBombInputResultSignature, AActor*, SourceActor, bool, bCorrect, FText, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombExplosionSignature, AActor*, SourceActor, FText, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombRuntimeResetSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombCheckpointRestoredSignature);

UCLASS(Blueprintable)
class ROOMESCAPE_API AREBombDefusalManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	AREBombDefusalManager();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombTimeChangedSignature OnBombTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombStepChangedSignature OnBombStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombInputResultSignature OnBombInputResult;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombExplosionSignature OnBombExploded;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombRuntimeResetSignature OnBombRuntimeReset;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombCheckpointRestoredSignature OnBombCheckpointRestored;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREBombPatternData> PatternData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	FName BadEndingCheckpointId = TEXT("BombBadEnding");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREPuzzleResetPoint>> BadEndingCheckpointPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float BadEndingFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float BadEndingBlackHoldSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float BadEndingFadeInSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 BadEndingFadeWidgetZOrder = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREFadeWidget> BadEndingFadeWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Feedback", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREBombFeedbackWidget> FeedbackWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Feedback", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float FeedbackWidgetDisplaySeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStepIndex, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 CurrentStepIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RemainingTimeSeconds, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	float RemainingTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREBombDevice> BombDevice;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREBombWire>> Wires;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREBombButton>> Buttons;

	UPROPERTY(Transient)
	TObjectPtr<UREBombFeedbackWidget> ActiveFeedbackWidget;

	UPROPERTY(Transient)
	TObjectPtr<UREFadeWidget> ActiveBadEndingFadeWidget;

	FTimerHandle BombTimerHandle;
	FTimerHandle BadEndingResetTimerHandle;
	FTimerHandle BadEndingFadeWidgetCleanupTimerHandle;

	// 서버 전용. 배드엔딩 위임 시 진행 매니저가 재활성화(StartActiveRound)할 때 페이드인/리셋 알림을 이어 보내기 위한 플래그.
	bool bBadEndingFlowDelegated = false;
	bool bPendingCheckpointRestoreNotify = false;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Bomb Defusal")
	void SetPatternData(UREBombPatternData* InPatternData);

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	UREBombPatternData* GetPatternData() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	int32 GetCurrentStepIndex() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	int32 GetTotalStepCount() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	float GetRemainingTimeSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	float GetTimeLimitSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Bad Ending")
	float GetBadEndingResetDelaySeconds() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	bool IsBombRunning() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	bool GetCurrentStep(FREBombStep& OutStep) const;

	void RegisterDevice(AREBombDevice* InDevice);
	void RegisterWire(AREBombWire* InWire);
	void RegisterButton(AREBombButton* InButton);

	bool SubmitWireCut(AREBombWire* Wire, AActor* Interactor);
	bool SubmitButtonToggle(AREBombButton* Button, AActor* Interactor);

	// 진행 매니저가 배드엔딩(텔레포트·리셋·재시작)을 대신 처리할 때 호출.
	// 위임되면 실패 시 내부 ResolveBadEnding 타이머를 걸지 않고 폭발 연출만 남긴다. (독립 테스트 레벨은 미위임 = 기존 자체 흐름)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Bomb Defusal|Bad Ending")
	void SetBadEndingFlowDelegated(bool bDelegated);

protected:
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;
	virtual void HandlePuzzleFailed() override;
	virtual void HandleSavedSolvedStateRestored() override;

	UFUNCTION()
	void OnRep_CurrentStepIndex();

	UFUNCTION()
	void OnRep_RemainingTimeSeconds();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombInputResult(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombExploded(AActor* SourceActor, const FText& ResultMessage);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombRuntimeReset();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombCheckpointRestored();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombInputResult(AActor* SourceActor, bool bCorrect, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombExploded(AActor* SourceActor, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombRuntimeReset();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombCheckpointRestored();

private:
	void StartActiveRound();
	void ResetRuntimeState(bool bResetDeviceState);
	void ApplyPatternToRegisteredActors();
	void StartTimer();
	void StopTimer();
	void TickTimer();
	void SetCurrentStepIndex(int32 NewStepIndex);
	void SetRemainingTimeSeconds(float NewRemainingTimeSeconds);
	void AdvanceStep(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage);
	void CompleteBomb(AActor* SourceActor, AActor* Interactor);
	void FailBomb(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage);
	void ResolveBadEnding();
	void TeleportPlayersToBadEndingCheckpoints();
	void ResolveBadEndingCheckpointPoints(TArray<AREPuzzleResetPoint*>& OutCheckpointPoints) const;
	void PlayBadEndingFadeOutLocal();
	void PlayBadEndingFadeInLocal();
	void CleanupBadEndingFadeWidgetLocal();
	void ShowFeedbackWidgetLocal(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage);
	APlayerController* ResolveLocalPlayerController(APlayerState* TargetPlayerState) const;
	FText BuildSuccessFeedbackMessage() const;
	FText BuildSolvedFeedbackMessage() const;
	FText BuildFailureFeedbackMessage() const;
	bool CanAcceptInput() const;
	bool ValidateCurrentWireStep(const AREBombWire* Wire, FText& OutFailureMessage) const;
	bool ValidateCurrentButtonStep(const AREBombButton* Button, bool bNextPressed, FText& OutFailureMessage) const;
	APlayerState* ResolvePlayerState(AActor* Actor) const;
};
