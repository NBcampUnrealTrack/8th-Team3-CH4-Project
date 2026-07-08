// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeTrapTile.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AGlassMazeManager;

/**
 * 함정 타일. 밟으면(서버 판정) 매니저에 통보 -> 매니저가 리셋 페널티 처리.
 * 바닥 색 힌트는 BP에서 TileMesh 머티리얼로.
 */
UCLASS()
class ROOMESCAPE_API AMazeTrapTile : public AActor
{
	GENERATED_BODY()

public:
	AMazeTrapTile();

	// 발동 연출 (유리 깨지는 소리 등) - 전원에게
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_OnTriggered();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "Maze", meta = (DisplayName = "On Triggered"))
	void BP_OnTriggered();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	TObjectPtr<UStaticMeshComponent> TileMesh;

	UPROPERTY(VisibleAnywhere, Category = "Maze")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditInstanceOnly, Category = "Maze")
	TObjectPtr<AGlassMazeManager> Manager;
};
