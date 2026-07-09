#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REDialLockDevice.generated.h"

class UStaticMeshComponent;
class UREDialLockWidget;
class ARELockAndGlowClueManager;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREDialLockDigitsChangedSignature, const TArray<int32>&, CurrentDigits);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREDialLockUnlockedChangedSignature, bool, bUnlocked);

UCLASS(Blueprintable)
class ROOMESCAPE_API AREDialLockDevice : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREDialLockDevice();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Dial Lock")
	FREDialLockDigitsChangedSignature OnDialLockDigitsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Dial Lock")
	FREDialLockUnlockedChangedSignature OnDialLockUnlockedChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dial Lock", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LockMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dial Lock|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREDialLockWidget> DialLockWidgetClass;

	UPROPERTY(ReplicatedUsing = OnRep_Digits, BlueprintReadOnly, Category = "Dial Lock|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<int32> CurrentDigits;

	UPROPERTY(ReplicatedUsing = OnRep_CodeLength, BlueprintReadOnly, Category = "Dial Lock|Runtime", meta = (AllowPrivateAccess = "true"))
	int32 RuntimeCodeLength = 4;

	UPROPERTY(ReplicatedUsing = OnRep_Unlocked, BlueprintReadOnly, Category = "Dial Lock|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bUnlocked = false;

	UPROPERTY(Transient)
	TObjectPtr<UREDialLockWidget> ActiveDialLockWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void SetLockAndGlowManager(ARELockAndGlowClueManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Dial Lock")
	ARELockAndGlowClueManager* GetLockAndGlowManager() const;

	UFUNCTION(BlueprintPure, Category = "Dial Lock")
	void GetCurrentDigits(TArray<int32>& OutDigits) const;

	UFUNCTION(BlueprintPure, Category = "Dial Lock")
	int32 GetCodeLength() const;

	UFUNCTION(BlueprintPure, Category = "Dial Lock")
	bool IsUnlocked() const;

	void ApplyServerCodeLength(int32 NewCodeLength);
	void ApplyServerUnlockedState(bool bNewUnlocked);

	UFUNCTION(Server, Reliable)
	void ServerIncrementDigit(int32 DigitIndex);

	UFUNCTION(Server, Reliable)
	void ServerDecrementDigit(int32 DigitIndex);

	UFUNCTION(Server, Reliable)
	void ServerSetDigit(int32 DigitIndex, int32 DigitValue);

	UFUNCTION(Server, Reliable)
	void ServerSubmitCode(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Dial Lock")
	void CloseDialLockWidget();

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION()
	void OnRep_Digits();

	UFUNCTION()
	void OnRep_CodeLength();

	UFUNCTION()
	void OnRep_Unlocked();

	UFUNCTION(Client, Reliable)
	void ClientOpenDialLockWidget(TSubclassOf<UREDialLockWidget> InWidgetClass, const TArray<int32>& InitialDigits);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock")
	void ReceiveDigitsChanged(const TArray<int32>& NewDigits);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dial Lock")
	void ReceiveUnlockedChanged(bool bNewUnlocked);

private:
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	void OpenDialLockWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREDialLockWidget> InWidgetClass, const TArray<int32>& InitialDigits);
	void NormalizeDigits();
	void SetDigitInternal(int32 DigitIndex, int32 DigitValue);
};
