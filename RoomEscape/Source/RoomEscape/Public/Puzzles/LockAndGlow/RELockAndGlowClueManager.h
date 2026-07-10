#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleManager.h"
#include "RELockAndGlowClueManager.generated.h"

class AREDialLockDevice;
class AREGlowPaintClueActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRELockAndGlowInputResultSignature, AActor*, SourceActor, bool, bCorrect, FText, ResultMessage);

UENUM(BlueprintType)
enum class ERELockAndGlowCorrectCodePolicy : uint8
{
	SolvePuzzle UMETA(DisplayName = "Solve Puzzle"),
	ValidateOnly UMETA(DisplayName = "Validate Only / Prototype")
};

UCLASS(Blueprintable)
class ROOMESCAPE_API ARELockAndGlowClueManager : public AREPuzzleManager
{
	GENERATED_BODY()

public:
	ARELockAndGlowClueManager();

	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Lock And Glow")
	FRELockAndGlowInputResultSignature OnDialLockInputResult;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Rule", meta = (ClampMin = "1", ClampMax = "8", AllowPrivateAccess = "true"))
	int32 CodeLength = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Rule", meta = (AllowPrivateAccess = "true"))
	FString SolutionCode = TEXT("0000");

	/*
	 * SolvePuzzle is the shipping behavior: correct input solves the puzzle and can
	 * be persisted by the common SaveGame progress system.
	 * ValidateOnly keeps the puzzle active after correct input, so the lock can be
	 * opened and tested repeatedly during prototype work without touching saved progress.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Rule", meta = (AllowPrivateAccess = "true"))
	ERELockAndGlowCorrectCodePolicy CorrectCodePolicy = ERELockAndGlowCorrectCodePolicy::SolvePuzzle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Prototype", meta = (AllowPrivateAccess = "true"))
	bool bDisableProgressPersistenceWhenValidateOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Feedback", meta = (AllowPrivateAccess = "true"))
	FText CorrectCodeMessage = FText::FromString(TEXT("자물쇠가 열렸습니다."));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock And Glow|Feedback", meta = (AllowPrivateAccess = "true"))
	FText IncorrectCodeMessage = FText::FromString(TEXT("번호가 맞지 않습니다."));

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lock And Glow|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREDialLockDevice>> LockDevices;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lock And Glow|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AREGlowPaintClueActor>> GlowClues;

public:
	UFUNCTION(BlueprintPure, Category = "Lock And Glow")
	int32 GetCodeLength() const;

	UFUNCTION(BlueprintPure, Category = "Lock And Glow")
	FString GetSolutionCode() const;

	UFUNCTION(BlueprintPure, Category = "Lock And Glow")
	bool IsValidateOnlyMode() const;

	UFUNCTION(BlueprintCallable, Category = "Lock And Glow")
	void BuildSolutionDigits(TArray<int32>& OutDigits) const;

	void RegisterLockDevice(AREDialLockDevice* InLockDevice);
	void RegisterGlowClue(AREGlowPaintClueActor* InGlowClue);

	bool SubmitCode(AREDialLockDevice* SourceDevice, AActor* Interactor, const TArray<int32>& SubmittedDigits);

protected:
	virtual void HandlePuzzleActivated() override;
	virtual void HandlePuzzleLocked() override;
	virtual void HandlePuzzleSolved() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDialLockInputResult(AActor* SourceActor, bool bCorrect, const FText& ResultMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lock And Glow")
	void ReceiveDialLockInputResult(AActor* SourceActor, bool bCorrect, const FText& ResultMessage);

private:
	FString NormalizeSolutionCode() const;
	bool AreDigitsMatchingSolution(const TArray<int32>& SubmittedDigits) const;
	void ApplyRuntimeStateToActors();
};
