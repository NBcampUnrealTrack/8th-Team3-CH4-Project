#pragma once

#include "CoreMinimal.h"
#include "Interaction/REInteractable.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "REBombDefusalManager.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UUserWidget;
class UWidgetComponent;
class UREBombPatternData;
class UREBombWireComponent;
class UREBombButtonComponent;
class UREBombDefusalWidget;
class UREBombFeedbackWidget;
class UREFadeWidget;
class AREPuzzleResetPoint;
class APlayerController;
class APlayerState;
class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombTimeChangedSignature, float, RemainingTimeSeconds, float, TimeLimitSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombStepChangedSignature, int32, CurrentStepIndex, int32, TotalStepCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FREBombInputResultSignature, AActor*, SourceActor, bool, bCorrect, FText, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FREBombExplosionSignature, AActor*, SourceActor, FText, ResultMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombRuntimeResetSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombCheckpointRestoredSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREBombElementStatesChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREBombPatternChangedSignature, UREBombPatternData*, NewPatternData);

/**
 * Integrated bomb actor.
 *
 * Create one Blueprint from this class (for example BP_BombDefusal), assign the
 * body mesh and PatternData, then add UREBombWireComponent and
 * UREBombButtonComponent children directly in the same Blueprint.  Interacting
 * with this actor opens the configured WBP; the WBP submits a WireIndex or
 * ButtonId back to this actor and the server validates it against PatternData.
 *
 * All bomb visuals and interactions live on this actor through
 * UREBombWireComponent / UREBombButtonComponent.  There is no separate
 * device, wire, or button actor path.
 */
UCLASS(Blueprintable)
class ROOMESCAPE_API AREBombDefusalManager : public AREPuzzleManager, public IREInteractable
{
	GENERATED_BODY()

public:
	AREBombDefusalManager();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	/** Fired whenever a cut-wire or pressed-button state changes/reset/replicates. */
	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombElementStatesChangedSignature OnBombElementStatesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bomb Defusal")
	FREBombPatternChangedSignature OnBombPatternChanged;

protected:
	// ---------------------------------------------------------------------
	// Integrated actor components: body, interaction, native display/prompt.
	// ---------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DeviceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> TimerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> StepText;

	/** Screen-space WBP presenter for the local interaction prompt. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> InteractionPromptWidgetComponent;

	/** Native world-text fallback used only when no prompt WBP is assigned. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Integrated", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> InteractionPromptTextRender;

	/** Assign the same prompt WBP used by other puzzle interactables. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FText InteractionPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	FVector InteractionPromptRelativeLocation = FVector(0.0, 0.0, 140.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	FVector2D InteractionPromptDrawSize = FVector2D(220.0, 64.0);

	/** When true, native TextRender is shown if no prompt WBP is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (AllowPrivateAccess = "true"))
	bool bUseNativePromptFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction Prompt", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float InteractionPromptWorldSize = 28.0f;

	/** Server-side safety check for WBP requests. Set to 0 to disable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Interaction", meta = (ClampMin = "0.0", Units = "cm", AllowPrivateAccess = "true"))
	float MaxWidgetInteractionDistance = 500.0f;

	// ---------------------------------------------------------------------
	// Data and WBP.
	// ---------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_PatternData, Category = "Bomb Defusal|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREBombPatternData> PatternData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREBombDefusalWidget> BombDefusalWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|UI", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 BombDefusalWidgetZOrder = 100;

	/** Usually false because WBP_BombDefusal has its own TXT_Feedback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|UI", meta = (AllowPrivateAccess = "true"))
	bool bShowStandaloneFeedbackWhileBombWidgetOpen = false;

	// ---------------------------------------------------------------------
	// Existing failure/reset flow.
	// ---------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	FName BadEndingCheckpointId = TEXT("BombBadEnding");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREPuzzleResetPoint>> BadEndingCheckpointPoints;

	/**
	 * Time TXT_Feedback remains fully readable before the bad-ending fade begins.
	 * The reset contract includes this hold, fade-out, and black-hold duration,
	 * so neither the internal nor delegated flow can remove the WBP early.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.1", UIMin = "0.1", Units = "s", AllowPrivateAccess = "true"))
	float FailureFeedbackHoldSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float BadEndingFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float BadEndingBlackHoldSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0.0", Units = "s", AllowPrivateAccess = "true"))
	float BadEndingFadeInSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 BadEndingFadeWidgetZOrder = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Bad Ending", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREFadeWidget> BadEndingFadeWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Feedback", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREBombFeedbackWidget> FeedbackWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bomb Defusal|Feedback", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float FeedbackWidgetDisplaySeconds = 0.0f;

	// ---------------------------------------------------------------------
	// Replicated runtime state.
	// ---------------------------------------------------------------------
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStepIndex, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 CurrentStepIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RemainingTimeSeconds, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	float RemainingTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RuntimeElementStates, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<int32> CutWireIndices;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RuntimeElementStates, Category = "Bomb Defusal|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FName> PressedButtonIds;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UREBombWireComponent>> IntegratedWireComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UREBombButtonComponent>> IntegratedButtonComponents;

	UPROPERTY(Transient)
	TObjectPtr<UREBombDefusalWidget> ActiveBombDefusalWidget;

	UPROPERTY(Transient)
	TObjectPtr<UREBombFeedbackWidget> ActiveFeedbackWidget;

	UPROPERTY(Transient)
	TObjectPtr<UREFadeWidget> ActiveBadEndingFadeWidget;

	FTimerHandle BombTimerHandle;
	FTimerHandle BadEndingResetTimerHandle;
	FTimerHandle BadEndingFadeOutDelayTimerHandle;
	FTimerHandle BadEndingFadeWidgetCleanupTimerHandle;

	TArray<TWeakObjectPtr<AActor>> LocalPromptCandidates;

	// Server-only flags for delegated bad-ending flow.
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

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Elements")
	bool GetWireDefinition(int32 WireIndex, FREBombWireDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Elements")
	bool GetButtonDefinition(FName ButtonId, FREBombButtonDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Elements")
	bool IsWireCut(int32 WireIndex) const;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Elements")
	bool IsButtonPressed(FName ButtonId) const;

	/** Server-side entry used by the owning PlayerController RPC. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Bomb Defusal|UI")
	bool SubmitWireSelectionFromWidget(int32 WireIndex, AActor* Interactor);

	/** Server-side entry used by the owning PlayerController RPC. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Bomb Defusal|UI")
	bool SubmitButtonSelectionFromWidget(FName ButtonId, AActor* Interactor);

	/** Called on the local client by AREPlayerController::ClientOpenBombDefusalWidget. */
	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal|UI")
	void OpenBombDefusalWidgetForLocalPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Bomb Defusal|UI")
	void CloseBombDefusalWidget();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Bomb Defusal|Integrated")
	void RefreshIntegratedBombComponents();

	/** Interface entry point. The player interaction RPC invokes this on the server. */
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Bomb Defusal|Interaction")
	bool CanInteractWithBomb(AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Bomb Defusal|Bad Ending")
	void SetBadEndingFlowDelegated(bool bDelegated);

protected:
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;
	virtual void HandlePuzzleFailed() override;
	virtual void HandleSavedSolvedStateRestored() override;
	virtual void OnRep_State() override;

	UFUNCTION()
	void OnRep_PatternData();

	UFUNCTION()
	void OnRep_CurrentStepIndex();

	UFUNCTION()
	void OnRep_RemainingTimeSeconds();

	UFUNCTION()
	void OnRep_RuntimeElementStates();

	UFUNCTION()
	void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal|Elements")
	void ReceiveBombElementStatesChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Defusal|Data")
	void ReceiveBombPatternChanged(UREBombPatternData* NewPatternData);

private:
	void StartActiveRound();
	void ResetRuntimeState();
	void ResetRuntimeElementStatesFromPattern();
	void ApplyDefinitionsToIntegratedComponents();
	void ApplyRuntimeStatesToIntegratedComponents();
	void CacheIntegratedComponents();
	void StartTimer();
	void StopTimer();
	void TickTimer();
	void SetCurrentStepIndex(int32 NewStepIndex);
	void SetRemainingTimeSeconds(float NewRemainingTimeSeconds);
	void SetWireCutState(int32 WireIndex, bool bNewCut);
	void SetButtonPressedState(FName ButtonId, bool bNewPressed);
	bool SubmitWireCutByIndex(int32 WireIndex, AActor* SourceActor, AActor* Interactor);
	bool SubmitButtonToggleById(FName ButtonId, AActor* SourceActor, AActor* Interactor);
	void AdvanceStep(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage);
	void CompleteBomb(AActor* SourceActor, AActor* Interactor);
	void FailBomb(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage);
	void ResolveBadEnding();
	void TeleportPlayersToBadEndingCheckpoints();
	void ResolveBadEndingCheckpointPoints(TArray<AREPuzzleResetPoint*>& OutCheckpointPoints) const;
	void ScheduleBadEndingFadeOutLocal();
	void PlayBadEndingFadeOutLocal();
	void PlayBadEndingFadeInLocal();
	void CleanupBadEndingFadeWidgetLocal();
	void ShowFeedbackWidgetLocal(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage);
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	APlayerController* ResolveLocalPlayerController(APlayerState* TargetPlayerState) const;
	FText BuildSuccessFeedbackMessage() const;
	FText BuildSolvedFeedbackMessage() const;
	FText BuildFailureFeedbackMessage() const;
	bool CanAcceptInput() const;
	bool CanSubmitInputFromInteractor(AActor* Interactor) const;
	bool ValidateIntegratedConfiguration(bool bLogWarnings) const;
	bool ValidateCurrentWireStep(int32 WireIndex, FText& OutFailureMessage) const;
	bool ValidateCurrentButtonStep(FName ButtonId, bool bNextPressed, FText& OutFailureMessage) const;
	APlayerState* ResolvePlayerState(AActor* Actor) const;
	void RefreshNativeDisplay();
	void SynchronizePromptCandidatesFromCurrentOverlaps();
	void RefreshInteractionPromptVisibility();
	void SetInteractionPromptVisible(bool bVisible);
	void RefreshInteractionPromptText() const;
	bool ShouldTrackPromptActor(AActor* Actor) const;
	bool ShouldShowPromptForActor(AActor* Actor) const;
};
