#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "REBombDevice.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class AREBombDefusalManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREBombDevice : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	AREBombDevice();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DeviceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> TimerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> StepText;

	UPROPERTY(ReplicatedUsing = OnRep_DeviceDisplay, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	float DisplayRemainingTimeSeconds = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_DeviceDisplay, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	float DisplayTimeLimitSeconds = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_DeviceDisplay, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	int32 DisplayCurrentStepIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_DeviceDisplay, BlueprintReadOnly, Category = "Bomb Device", meta = (AllowPrivateAccess = "true"))
	int32 DisplayTotalStepCount = 0;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Device")
	void SetBombManager(AREBombDefusalManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Bomb Device")
	AREBombDefusalManager* GetBombManager() const;

	void ApplyServerTime(float RemainingTimeSeconds, float TimeLimitSeconds);
	void ApplyServerStep(int32 CurrentStepIndex, int32 TotalStepCount);

protected:
	UFUNCTION()
	void OnRep_DeviceDisplay();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Device")
	void ReceiveDeviceDisplayChanged(float RemainingTimeSeconds, float TimeLimitSeconds, int32 CurrentStepIndex, int32 TotalStepCount);

private:
	void RefreshNativeText();
};
