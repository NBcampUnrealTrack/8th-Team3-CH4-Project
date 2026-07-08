// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlassWall.generated.h"

class UStaticMeshComponent;
class AGlassMazeManager;

/**
 * 개폐형 유리벽. 레벨 디자이너가 레벨에 직접 배치하고 Manager/LeverID를 인스턴스별로 지정한다.
 * 콜리전 전환은 C++(즉시), 슬라이드 연출은 BP 타임라인으로 분리.
 */
UCLASS()
class ROOMESCAPE_API AGlassWall : public AActor
{
	GENERATED_BODY()

public:
	AGlassWall();

	// 서버 전용. 닫을 때 벽 자리에 플레이어가 있으면 무시하고 false 반환
	bool SetOpen(bool bNewOpen);

	bool IsOpen() const { return bIsOpen; }

	UPROPERTY(EditInstanceOnly, Category = "Maze")
	TObjectPtr<AGlassMazeManager> Manager;

	UPROPERTY(EditAnywhere, Category = "Maze")
	FName LeverID = NAME_None;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_IsOpen();

	// 슬라이드/페이드 연출은 BP에서 (WallOpenTime 길이의 타임라인 권장)
	UFUNCTION(BlueprintImplementableEvent, Category = "Maze", meta = (DisplayName = "On Open State Changed"))
	void BP_OnOpenStateChanged(bool bNewOpen);

	void ApplyCollision();
	bool IsBlockedByPawn() const;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
	bool bIsOpen = false;

	// 개폐 연출 길이 (BP 타임라인에서 참조)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maze")
	float WallOpenTime = 0.8f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
	TObjectPtr<UStaticMeshComponent> WallMesh;
};
