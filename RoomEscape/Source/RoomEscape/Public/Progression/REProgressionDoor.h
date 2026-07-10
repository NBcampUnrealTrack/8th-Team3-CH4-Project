// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "REProgressionDoor.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREProgressionDoorStateSignature, bool, bNewIsOpen);

/**
 * 진행 게이트 문. 서버에서 SetOpen → bIsOpen 복제 → 양쪽에서 콜리전/비주얼 반영.
 * 기본 동작: 열리면 콜리전 해제 + 메쉬 숨김(그레이박스용).
 * 연출(스윙 애니/사운드)이 필요하면 bHideMeshWhenOpen을 끄고 ReceiveDoorStateChanged를 BP에서 구현.
 */
UCLASS(Blueprintable)
class ROOMESCAPE_API AREProgressionDoor : public AActor
{
	GENERATED_BODY()

public:
	AREProgressionDoor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Progression Door")
	FREProgressionDoorStateSignature OnDoorStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression Door", meta = (AllowPrivateAccess = "true"))
	bool bHideMeshWhenOpen = true;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, BlueprintReadOnly, Category = "Progression Door", meta = (AllowPrivateAccess = "true"))
	bool bIsOpen = false;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Progression Door")
	void SetOpen(bool bNewIsOpen);

	UFUNCTION(BlueprintPure, Category = "Progression Door")
	bool IsOpen() const;

protected:
	UFUNCTION()
	void OnRep_IsOpen();

	UFUNCTION(BlueprintImplementableEvent, Category = "Progression Door")
	void ReceiveDoorStateChanged(bool bNewIsOpen);

	void ApplyDoorState();
};
