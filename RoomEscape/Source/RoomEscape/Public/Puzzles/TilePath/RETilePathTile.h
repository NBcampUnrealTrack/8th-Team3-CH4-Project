#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "Puzzles/TilePath/RETilePathTypes.h"
#include "RETilePathTile.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
class UMaterialInterface;
class ARETilePathManager;
class APawn;

UCLASS(Blueprintable)
class ROOMESCAPE_API ARETilePathTile : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	ARETilePathTile();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Tile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Path Tile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> StepCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Collision", meta = (AllowPrivateAccess = "true"))
	FVector StepCollisionExtent = FVector(45.0, 45.0, 80.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Collision", meta = (AllowPrivateAccess = "true"))
	FVector2D StepAcceptanceHalfExtent = FVector2D(60.0, 60.0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Collision", meta = (AllowPrivateAccess = "true"))
	float MinimumStepLocalZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Collision", meta = (AllowPrivateAccess = "true"))
	float MaximumStepLocalZ = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_TileCoordinate, Category = "Tile Path Tile", meta = (AllowPrivateAccess = "true"))
	FIntPoint TileCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> LockedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> AvailableMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> CurrentMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> PassedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> FailedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> SolvedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	bool bMoveMeshByState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	float LockedMeshZOffset = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tile Path Tile|Visual", meta = (AllowPrivateAccess = "true"))
	float ActiveMeshZOffset = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_TileState, BlueprintReadOnly, Category = "Tile Path Tile|Runtime", meta = (AllowPrivateAccess = "true"))
	ERETilePathTileState TileState = ERETilePathTileState::Default;

	UPROPERTY(ReplicatedUsing = OnRep_Walkable, BlueprintReadOnly, Category = "Tile Path Tile|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bWalkable = true;

public:
	UFUNCTION(BlueprintCallable, Category = "Tile Path Tile")
	void SetTilePathManager(ARETilePathManager* InManager);

	UFUNCTION(BlueprintCallable, Category = "Tile Path Tile")
	void SetTileCoordinate(FIntPoint InTileCoordinate);

	UFUNCTION(BlueprintPure, Category = "Tile Path Tile")
	FIntPoint GetTileCoordinate() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Tile")
	ARETilePathManager* GetTilePathManager() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Tile")
	ERETilePathTileState GetTileState() const;

	UFUNCTION(BlueprintPure, Category = "Tile Path Tile")
	bool IsWalkable() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tile Path Tile")
	void ApplyServerTileState(ERETilePathTileState NewTileState, bool bNewWalkable);

protected:

	UFUNCTION()
	void OnStepCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_TileCoordinate();

	UFUNCTION()
	void OnRep_TileState();

	UFUNCTION()
	void OnRep_Walkable();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tile Path Tile")
	void ReceiveTileStateChanged(ERETilePathTileState NewTileState, bool bNewWalkable);

private:
	void ApplyTileStateVisual();
	void ApplyWalkableCollision();
	bool IsPawnInStepAcceptanceArea(APawn* Pawn) const;
	UMaterialInterface* GetMaterialForState(ERETilePathTileState InTileState) const;
};
