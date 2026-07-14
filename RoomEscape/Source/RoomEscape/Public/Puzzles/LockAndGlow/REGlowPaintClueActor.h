#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "REGlowPaintClueActor.generated.h"

class ARELockAndGlowClueManager;
class AREPlayerCharacter;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREGlowPaintClueActor : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	AREGlowPaintClueActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	/*
	 * World-space plane that contains the hidden clue texture.
	 * At runtime the component stays hidden unless at least one real flashlight beam
	 * reaches its bounds. A dynamic material then masks the visible pixels to the
	 * actual spotlight cone.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Glow Clue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CluePlaneComponent;

	/*
	 * Material used to render the clue. For pixel-accurate beam reveal, the material
	 * must implement the GlowReveal_* parameters documented with this patch.
	 * Without those parameters the C++ cone test still provides an actor-level fallback.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> VisibleEmissiveMaterial;

	/*
	 * A flashlight is considered only while its owning character is inside this distance
	 * from the clue. This is intentionally independent from the spotlight attenuation
	 * radius, so a long-range flashlight cannot reveal the clue from across the room.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Flashlight Reveal", meta = (ClampMin = "1.0", Units = "cm", AllowPrivateAccess = "true"))
	float FlashlightRecognitionRange = 500.0f;

	/*
	 * Prevents a flashlight in another room from revealing the clue through a wall.
	 * The visibility trace is visual-only and is evaluated independently on each client.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Flashlight Reveal", meta = (AllowPrivateAccess = "true"))
	bool bRequireLineOfSightToInfluencingFlashlight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Flashlight Reveal", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> OcclusionTraceChannel = ECC_Visibility;

	/* Moves the trace target slightly toward the flashlight to avoid the wall behind a clue plane. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Flashlight Reveal", meta = (ClampMin = "0.0", Units = "cm", AllowPrivateAccess = "true"))
	float OcclusionTraceEndOffset = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	bool bHideAfterPuzzleSolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Performance", meta = (ClampMin = "0.02", ClampMax = "0.25", AllowPrivateAccess = "true"))
	float VisibilityRefreshInterval = 0.03f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicRevealMaterial;

	bool bCurrentlyVisible = false;
	bool bClueVisibilityInitialized = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void SetLockAndGlowManager(ARELockAndGlowClueManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	ARELockAndGlowClueManager* GetLockAndGlowManager() const;

	/* True when at least one unobstructed flashlight beam currently reaches this clue. */
	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	bool IsClueVisible() const;

	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	UStaticMeshComponent* GetCluePlaneComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void SetVisibleEmissiveMaterial(UMaterialInterface* NewMaterial);

	/* Kept as the public refresh entry point used by the puzzle manager and existing Blueprints. */
	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void RefreshClueVisibility();

protected:
	virtual void HandlePuzzleManagerChanged() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Glow Clue")
	void ReceiveClueVisibilityChanged(bool bNewVisible);

private:
	struct FFlashlightRevealData
	{
		TWeakObjectPtr<AREPlayerCharacter> SourceCharacter;
		FVector Position = FVector::ZeroVector;
		FVector Direction = FVector::ForwardVector;
		float Range = 0.0f;
		float InnerConeCos = 1.0f;
		float OuterConeCos = 1.0f;
	};

	bool ShouldBeVisibleByPuzzleState() const;
	void CollectAffectingFlashlights(TArray<FFlashlightRevealData>& OutFlashlights) const;
	bool BuildFlashlightRevealData(AREPlayerCharacter* PlayerCharacter, FFlashlightRevealData& OutData) const;
	bool IsSourceCharacterWithinRecognitionRange(const FFlashlightRevealData& FlashlightData) const;
	bool DoesFlashlightConeReachClue(const FFlashlightRevealData& FlashlightData) const;
	bool HasUnobstructedFlashlightPath(const FFlashlightRevealData& FlashlightData) const;

	void ApplyClueMaterial();
	void EnsureDynamicRevealMaterial();
	void UpdateMaterialRevealParameters(bool bPuzzleAllowsReveal, const TArray<FFlashlightRevealData>& Flashlights);
	void SetMaterialFlashlightParameters(int32 LightIndex, const FFlashlightRevealData* FlashlightData);
	void ApplyClueVisibility(bool bNewVisible);
};
