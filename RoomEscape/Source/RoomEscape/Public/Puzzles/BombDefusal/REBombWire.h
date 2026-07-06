#pragma once

#include "CoreMinimal.h"
#include "Puzzles/BombDefusal/REBombDefusalTypes.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REBombWire.generated.h"

class UStaticMeshComponent;
class AREBombDefusalManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREBombWire : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREBombWire();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WireMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_WireVisualState, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	int32 WireIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_WireVisualState, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	EREBombWireColor WireColor = EREBombWireColor::Red;

	UPROPERTY(ReplicatedUsing = OnRep_WireCut, BlueprintReadOnly, Category = "Bomb Wire", meta = (AllowPrivateAccess = "true"))
	bool bCut = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Bomb Wire")
	void SetBombManager(AREBombDefusalManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	AREBombDefusalManager* GetBombManager() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	int32 GetWireIndex() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	EREBombWireColor GetWireColor() const;

	UFUNCTION(BlueprintPure, Category = "Bomb Wire")
	bool IsCut() const;

	void ApplyServerWireDefinition(const FREBombWireDefinition& WireDefinition);
	void ApplyServerCutState(bool bNewCut);

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION()
	void OnRep_WireCut();

	UFUNCTION()
	void OnRep_WireVisualState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Wire")
	void ReceiveWireCutChanged(bool bNewCut);

	UFUNCTION(BlueprintImplementableEvent, Category = "Bomb Wire")
	void ReceiveWireVisualChanged(int32 NewWireIndex, EREBombWireColor NewWireColor);

private:
	void RefreshWireVisuals();
};
