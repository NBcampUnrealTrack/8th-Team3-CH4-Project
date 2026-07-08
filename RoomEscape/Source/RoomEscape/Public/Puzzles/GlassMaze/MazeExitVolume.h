// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeExitVolume.generated.h"

class UBoxComponent;
class AGlassMazeManager;

/** B가 도달하면 매니저에 클리어를 통보하는 볼륨. 판정은 서버에서만. */
UCLASS()
class ROOMESCAPE_API AMazeExitVolume : public AActor
{
	GENERATED_BODY()

public:
	AMazeExitVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Maze")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditInstanceOnly, Category = "Maze")
	TObjectPtr<AGlassMazeManager> Manager;
};
