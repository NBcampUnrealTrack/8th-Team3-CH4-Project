#pragma once

#include "CoreMinimal.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "REDefusalManual.generated.h"

class UStaticMeshComponent;
class UREBombManualWidget;
class UREBombPatternData;
class AREBombDefusalManager;
class APlayerController;

UCLASS(Blueprintable)
class ROOMESCAPE_API AREDefusalManual : public AREPuzzleInteractableActor
{
	GENERATED_BODY()

public:
	AREDefusalManual();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defusal Manual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ManualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defusal Manual", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UREBombManualWidget> ManualWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defusal Manual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UREBombPatternData> OverridePatternData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defusal Manual", meta = (AllowPrivateAccess = "true"))
	FText ManualTitle = FText::FromString(TEXT("해제 설명서"));

	UPROPERTY(Transient)
	TObjectPtr<UREBombManualWidget> ActiveManualWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "Defusal Manual")
	void SetBombManager(AREBombDefusalManager* InManager);

	UFUNCTION(BlueprintPure, Category = "Defusal Manual")
	AREBombDefusalManager* GetBombManager() const;

	UFUNCTION(BlueprintPure, Category = "Defusal Manual")
	UREBombPatternData* GetManualPatternData() const;

	UFUNCTION(BlueprintCallable, Category = "Defusal Manual")
	void CloseManualWidget();

protected:
	virtual bool CanUseElement(AActor* Interactor) const override;
	virtual void HandleInteract(AActor* Interactor) override;

	UFUNCTION(Client, Reliable)
	void ClientOpenManualWidget(APlayerController* OwningPlayerController, TSubclassOf<UREBombManualWidget> InWidgetClass, const FText& InManualTitle, const FText& InManualText);

private:
	APlayerController* ResolvePlayerController(AActor* Actor) const;
	void OpenManualWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREBombManualWidget> InWidgetClass, const FText& InManualTitle, const FText& InManualText);
};
