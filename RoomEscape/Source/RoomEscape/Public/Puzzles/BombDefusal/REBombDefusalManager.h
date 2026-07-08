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
class APlayerController;
class APlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombTimeChangedSignature, float, RemainingTimeSeconds, float, TimeLimitSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombStepChangedSignature, int32, CurrentStepIndex, int32, TotalStepCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FREBombInputResultSignature, AActor*, SourceActor, bool, bCorrect, FText, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombRuntimeResetSignature);

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
	FREBombRuntimeResetSignature OnBombRuntimeReset;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREBombPatternData> PatternData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Rule", meta = (AllowPrivateAccess = "true"))
	bool bAutoResetOnFailure = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Rule", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ResetDelaySeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Rule", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float FailureRestrictionSeconds = 60.0f;

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

	FTimerHandle BombTimerHandle;
	FTimerHandle FailureResetTimerHandle;

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

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	float GetFailureRestrictionSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	bool IsBombRunning() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal")
	bool GetCurrentStep(FREBombStep& OutStep) const;

	void RegisterDevice(AREBombDevice* InDevice);
	void RegisterWire(AREBombWire* InWire);
	void RegisterButton(AREBombButton* InButton);

	bool SubmitWireCut(AREBombWire* Wire, AActor* Interactor);
	bool SubmitButtonToggle(AREBombButton* Button, AActor* Interactor);

protected:
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;
	virtual void HandlePuzzleFailed() override;

	UFUNCTION()
	void OnRep_CurrentStepIndex();

	UFUNCTION()
	void OnRep_RemainingTimeSeconds();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombInputResult(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBombRuntimeReset();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombInputResult(AActor* SourceActor, bool bCorrect, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal")
	void ReceiveBombRuntimeReset();

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
	void ResetAfterFailure();
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
