#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "REGlowPaintClueActor.generated.h"

class APawn;
class ARELockAndGlowClueManager;
class UMaterialInterface;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EREGlowClueVisibilityAuthority : uint8
{
	ServerRelevantFlashlights UMETA(DisplayName = "Server Relevant Flashlights"),
	LocalViewingPlayer UMETA(DisplayName = "Local Viewing Player")
};

UCLASS(Blueprintable)
class ROOMESCAPE_API AREGlowPaintClueActor : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	AREGlowPaintClueActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/*
	 * A simple world-space plane for the hidden number decal.
	 * Put a material instance made from M_GlowNumber on this component.
	 * The material should use a transparent number PNG alpha as Opacity Mask.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Glow Clue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CluePlaneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Material", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> VisibleEmissiveMaterial;

	/*
	 * ServerRelevantFlashlights keeps the clue as one shared world-state in multiplayer.
	 * If any relevant player's flashlight is on, every client sees the clue hidden.
	 * LocalViewingPlayer exists only for intentionally client-local clue behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	EREGlowClueVisibilityAuthority VisibilityAuthority = EREGlowClueVisibilityAuthority::ServerRelevantFlashlights;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	bool bVisibleOnlyWhenFlashlightOff = true;

	/*
	 * Prevents a clue from revealing itself just because every player is far enough
	 * away to be ignored by FlashlightInfluenceRadius. With this enabled, at least
	 * one relevant local/server player must be inside the influence area before the
	 * clue can become visible. This makes the game default to hidden and reveal
	 * only when someone is actually near the clue in darkness.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	bool bRequireRelevantPlayerToReveal = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	bool bEnableCollisionWhenVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (AllowPrivateAccess = "true"))
	bool bHideAfterPuzzleSolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility", meta = (ClampMin = "0.02", AllowPrivateAccess = "true"))
	float VisibilityRefreshInterval = 0.1f;

	/*
	 * Only flashlights inside this radius affect this clue.
	 * Keep this > 0 to avoid a flashlight in another room hiding every clue in the level.
	 * Set 0 if the clue should react to any flashlight in the current world.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility|Server Authority", meta = (ClampMin = "0.0", Units = "cm", AllowPrivateAccess = "true"))
	float FlashlightInfluenceRadius = 1500.0f;

	/*
	 * Optional room-aware filtering. Enable when walls should block another room's
	 * flashlight from affecting this clue.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glow Clue|Visibility|Server Authority", meta = (AllowPrivateAccess = "true"))
	bool bRequireLineOfSightToInfluencingFlashlight = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedClueVisible, Transient, BlueprintReadOnly, Category = "Glow Clue|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bReplicatedClueVisible = false;

	bool bCurrentlyVisible = false;
	bool bClueVisibilityInitialized = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void SetLockAndGlowManager(ARELockAndGlowClueManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	ARELockAndGlowClueManager* GetLockAndGlowManager() const;

	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	bool IsClueVisible() const;

	UFUNCTION(BlueprintPure, Category = "Glow Clue")
	UStaticMeshComponent* GetCluePlaneComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void SetVisibleEmissiveMaterial(UMaterialInterface* NewMaterial);

	UFUNCTION(BlueprintCallable, Category = "Glow Clue")
	void RefreshClueVisibility();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Glow Clue")
	void ReceiveClueVisibilityChanged(bool bNewVisible);

private:
	UFUNCTION()
	void OnRep_ReplicatedClueVisible();

	bool ShouldBeVisible() const;
	bool ShouldBeVisibleByPuzzleState() const;
	bool HasRelevantViewer() const;
	bool HasLocalRelevantViewer() const;
	bool IsLocalFlashlightOn() const;
	bool IsAnyRelevantFlashlightOn() const;
	bool IsPawnFlashlightOn(const APawn* Pawn) const;
	bool IsFlashlightRelevant(const APawn* Pawn) const;
	void SetAuthoritativeClueVisibility(bool bNewVisible);
	void ApplyClueMaterial();
	void ApplyClueVisibility(bool bNewVisible);
};
