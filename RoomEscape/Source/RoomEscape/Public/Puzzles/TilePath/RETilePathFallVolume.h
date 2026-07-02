#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "RETilePathFallVolume.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class ARETilePathManager;
class APawn;

UCLASS(Blueprintable)
class ROOMESCAPE_API ARETilePathFallVolume : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	ARETilePathFallVolume();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> FallCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	bool bRequirePawnBelowVolumeTop = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	float VolumeTopTolerance = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	bool bRequireDownwardVelocity = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	float RequiredDownwardVelocityZ = -10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Fall", meta = (AllowPrivateAccess = "true"))
	float MinimumReprocessIntervalSeconds = 0.5f;

	TSet<TWeakObjectPtr<APawn>> OverlappingPawns;
	TMap<TWeakObjectPtr<APawn>, double> LastProcessedTimes;

public:
	UFUNCTION(BlueprintCallable, Category = "Tile Path Fall")
	void SetTilePathManager(ARETilePathManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Tile Path Fall")
	ARETilePathManager* GetTilePathManager() const;

protected:
	void ProcessFallenPawn(APawn* FallingPawn);
	void ResolvePuzzleManagerIfNeeded();
	bool CanProcessFallingPawn(APawn* FallingPawn) const;
	void CleanupProcessedPawns();

	UFUNCTION()
	void OnFallCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFallCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
