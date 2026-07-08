#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleActor.h"
#include "Puzzles/MirrorRoom/REMirrorRoomTypes.h"
#include "REMirrorCue.generated.h"

class UTextRenderComponent;
class UStaticMeshComponent;
class UREMirrorPuzzleData;
class AREMirrorRoomManager;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREMirrorCue : public AREPuzzleActor
{
	GENERATED_BODY()

public:
	AREMirrorCue();
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> CueRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CueMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> CueTextRender;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	FName CueId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	int32 CueIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREMirrorPuzzleData> OverridePuzzleData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	FText FallbackDisplayText = FText::FromString(TEXT("?"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirror Cue", meta = (AllowPrivateAccess = "true"))
	float TextWorldSize = 64.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Mirror Cue")
	void SetMirrorManager(AREMirrorRoomManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Mirror Cue")
	AREMirrorRoomManager* GetMirrorManager() const;

	UFUNCTION(BlueprintPure, Category = "Mirror Cue")
	UREMirrorPuzzleData* GetResolvedPuzzleData() const;

	UFUNCTION(BlueprintCallable, Category = "Mirror Cue")
	void RefreshCueVisual();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Mirror Cue")
	void ReceiveCueVisualRefreshed(const FREMirrorCueDefinition& CueDefinition, const FText& DisplayText);

private:
	bool ResolveCueDefinition(FREMirrorCueDefinition& OutCueDefinition) const;
};
