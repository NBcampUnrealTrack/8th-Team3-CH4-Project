#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathManager.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UDataTable;
class ARETilePathTile;
class AREPuzzleHintActor;
class APlayerController;
class APlayerState;
class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRETilePathProgressChangedSignature, int32, CurrentStepIndex, int32, RevealedMoveCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRETilePathQuestionChangedSignature, FName, QuestionId, int32, QuestionIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRETilePathDirectionRevealedSignature, const FRETilePathRevealedMove&, RevealedMove);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRETilePathTileJudgedSignature, APawn*, Pawn, FIntPoint, TileCoordinate, bool, bCorrect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRETilePathPawnSignature, APawn*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRETilePathTimeChangedSignature, float, RemainingTimeSeconds, float, TimeLimitSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRETilePathSessionStateChangedSignature, bool, bStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRETilePathSessionPhaseChangedSignature, ERETilePathSessionPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRETilePathCountdownChangedSignature, int32, CountdownNumber, bool, bGameStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRETilePathParticipantsChangedSignature);

UCLASS(Blueprintable)
class ROOMESCAPE_API ARETilePathManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	ARETilePathManager();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathProgressChangedSignature OnTilePathProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathQuestionChangedSignature OnTilePathQuestionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathDirectionRevealedSignature OnTilePathDirectionRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathTileJudgedSignature OnTilePathTileJudged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathPawnSignature OnTilePathWalkerReset;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathPawnSignature OnTilePathCleared;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathTimeChangedSignature OnTilePathTimeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathSessionStateChangedSignature OnTilePathSessionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathSessionPhaseChangedSignature OnTilePathSessionPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathCountdownChangedSignature OnTilePathCountdownChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tile Path Puzzle")
	FRETilePathParticipantsChangedSignature OnTilePathParticipantsChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> PuzzleAreaBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Grid", meta = (ClampMin = "2", AllowPrivateAccess = "true"))
	int32 GridSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Grid", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float TileSpacing = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Grid", meta = (AllowPrivateAccess = "true"))
	bool bSpawnTilesOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Grid", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARETilePathTile> TileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Path", meta = (AllowPrivateAccess = "true"))
	TArray<FIntPoint> SafePath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Question", meta = (AllowPrivateAccess = "true", RequiredAssetDataTags = "RowStructure=/Script/RoomEscape.RETilePathQuestionRow"))
	TObjectPtr<UDataTable> QuestionDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Question", meta = (AllowPrivateAccess = "true"))
	TArray<FName> QuestionRowOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Rule", meta = (AllowPrivateAccess = "true"))
	bool bRequireRevealedMoveToStep = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Rule", meta = (AllowPrivateAccess = "true"))
	bool bAllowGuideToSolveBeforeStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Rule", meta = (AllowPrivateAccess = "true"))
	bool bAllowSamePlayerForBothRoles = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Rule", meta = (AllowPrivateAccess = "true"))
	bool bRequireWalkerInsidePuzzleArea = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Rule", meta = (AllowPrivateAccess = "true"))
	bool bKeepRevealedMovesAfterFailure = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Countdown", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
	int32 CountdownStartNumber = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Countdown", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float CountdownIntervalSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Countdown", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float GameStartDisplaySeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Tile Activation", meta = (AllowPrivateAccess = "true"))
	bool bGateTileCollisionByRevealedPath = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Tile Activation", meta = (AllowPrivateAccess = "true"))
	bool bRevealTileStateInWorld = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Timer", meta = (AllowPrivateAccess = "true"))
	bool bUseTimeLimit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Timer", meta = (ClampMin = "1.0", EditCondition = "bUseTimeLimit", AllowPrivateAccess = "true"))
	float TimeLimitSeconds = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Reset", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> ResetPointActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Reset", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ResetDelaySeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Reset", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float PostResetInputIgnoreSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Puzzle|Reward", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AREPuzzleHintActor> HintActor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStepIndex, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 CurrentStepIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RevealedMoveCount, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 RevealedMoveCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RemainingTimeSeconds, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	float RemainingTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ParticipantsChanged, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APawn> WalkerPawn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ParticipantsChanged, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APlayerState> WalkerPlayerState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ParticipantsChanged, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APlayerState> GuidePlayerState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_SessionStarted, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bSessionStarted = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_SessionPhase, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	ERETilePathSessionPhase SessionPhase = ERETilePathSessionPhase::Waiting;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CountdownNumber, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 CountdownNumber = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<APawn>> PawnsInsideArea;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tile Path Puzzle|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<ARETilePathTile>> TileActors;

	UPROPERTY(Transient)
	TObjectPtr<APawn> PendingResetPawn;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> GuideController;

	TArray<TWeakObjectPtr<APawn>> ResetProtectedPawns;
	TArray<double> ResetProtectionEndTimes;

	FTimerHandle PuzzleTimerHandle;
	FTimerHandle ResetTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle GameStartTimerHandle;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	bool StartPuzzle(APawn* InWalkerPawn);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	bool RegisterGuide(APlayerController* InGuideController);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	bool CancelGuide(APlayerController* InGuideController);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	bool CancelWalker(APawn* InWalkerPawn);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	bool SubmitAnswer(APlayerController* RequestingController, FName QuestionId, ERETilePathAnswerChoice SubmittedAnswer, FRETilePathRevealedMove& OutRevealedMove);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void HandleTileStepped(ARETilePathTile* Tile, APawn* SteppingPawn);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void HandleWalkerFell(APawn* FallingPawn);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void ResetPawnToResetPoint(APawn* PawnToReset);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void ResetTilePathPuzzle(bool bResetGuideProgress = true);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void RegisterTile(ARETilePathTile* Tile);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Puzzle")
	void RefreshTileStates();

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool CanStartPuzzle(APawn* InWalkerPawn) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool CanSubmitAnswer(APlayerController* RequestingController) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsSafePathValid() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsCoordinateInGrid(FIntPoint Coordinate) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	int32 GetTotalMoveCount() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool AreAllQuestionsSolved() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	int32 GetCurrentStepIndex() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	int32 GetRevealedMoveCount() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	float GetRemainingTimeSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	float GetTimeLimitSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsSessionStarted() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	ERETilePathSessionPhase GetSessionPhase() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	int32 GetCountdownNumber() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsCountdownActive() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsWalkerRegistered() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool IsGuideRegistered() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	APawn* GetWalkerPawn() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	APlayerState* GetGuidePlayerState() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	void GetSafePath(TArray<FIntPoint>& OutSafePath) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	void GetRegisteredTiles(TArray<ARETilePathTile*>& OutTiles) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	ARETilePathTile* FindTileByCoordinate(FIntPoint Coordinate) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool GetQuestionData(FName QuestionId, FRETilePathQuestionRow& OutQuestionRow) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	bool GetRevealedMoveByStep(int32 StepIndex, FRETilePathRevealedMove& OutRevealedMove) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	void GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Puzzle")
	FName GetCurrentQuestionId() const;

protected:
	virtual void OnRep_State() override;
	virtual bool CanActivatePuzzle() const override;
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;
	virtual void HandlePuzzleFailed() override;

	UFUNCTION()
	void OnRep_CurrentStepIndex();

	UFUNCTION()
	void OnRep_RevealedMoveCount();

	UFUNCTION()
	void OnRep_RemainingTimeSeconds();

	UFUNCTION()
	void OnRep_SessionStarted();

	UFUNCTION()
	void OnRep_SessionPhase();

	UFUNCTION()
	void OnRep_CountdownNumber();

	UFUNCTION()
	void OnRep_ParticipantsChanged();

	UFUNCTION()
	void OnPuzzleAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPuzzleAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleTileJudged(APawn* Pawn, FIntPoint TileCoordinate, bool bCorrect);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandlePuzzleFailed(APawn* Pawn, FIntPoint TileCoordinate);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleWalkerReset(APawn* Pawn);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandlePuzzleCleared(APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Puzzle")
	void ReceiveTileJudged(APawn* Pawn, FIntPoint TileCoordinate, bool bCorrect);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Puzzle")
	void ReceivePuzzleFailed(APawn* Pawn, FIntPoint TileCoordinate);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Puzzle")
	void ReceiveWalkerReset(APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Puzzle")
	void ReceivePuzzleCleared(APawn* Pawn);

private:
	void SpawnTiles();
	void SetCurrentStepIndex(int32 NewCurrentStepIndex);
	void SetRevealedMoveCount(int32 NewRevealedMoveCount);
	void SetRemainingTimeSeconds(float NewRemainingTimeSeconds);
	void StartPuzzleTimer();
	void TryStartSession();
	void StartSessionCountdown();
	void AdvanceSessionCountdown();
	void FinishSessionCountdown();
	void StopSessionCountdown();
	void SetSessionStarted(bool bNewSessionStarted);
	void SetSessionPhase(ERETilePathSessionPhase NewSessionPhase);
	void SetCountdownNumber(int32 NewCountdownNumber);
	void BroadcastParticipantsChanged();
	void StopPuzzleTimer();
	void OnPuzzleTimerTick();
	void FailAndReset(APawn* FailedPawn, FIntPoint FailedCoordinate);
	void FinishReset();
	void ClearPuzzle();
	void TeleportPawnToResetPoint(APawn* PawnToReset) const;
	void AddResetProtection(APawn* Pawn);
	bool IsResetProtected(APawn* Pawn) const;
	void CleanupResetProtection();
	bool IsPawnInsidePuzzleArea(APawn* Pawn) const;
	bool IsAdjacentStep(FIntPoint From, FIntPoint To) const;
	bool BuildRevealedMove(int32 StepIndex, FName QuestionId, FRETilePathRevealedMove& OutRevealedMove) const;
	ERETilePathDirection CalculateDirection(FIntPoint From, FIntPoint To) const;
	FTransform GetResetTransform() const;
	int32 FindSafePathIndex(FIntPoint Coordinate) const;
};
