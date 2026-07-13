// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/REInteractable.h"
#include "REPickupItemActor.generated.h"

class UItemDataAsset;
class UStaticMeshComponent;

// 픽업 아이템 부모 클래스 — 서버가 픽업 판정·월드 제거를 소유하고,
// 인벤토리 추가는 로컬(비복제) 컴포넌트이므로 소유 클라이언트에서 Client RPC로 수행
UCLASS(Abstract, Blueprintable)
class ROOMESCAPE_API AREPickupItemActor : public AActor, public IREInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AREPickupItemActor();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// 픽업 시 인벤토리에 추가할 아이템 데이터 (배치 인스턴스에서 지정 필수)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UItemDataAsset> ItemData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 픽업 완료 상태 — 복제 변수, OnRep에서 숨김/콜리전 해제
	UPROPERTY(ReplicatedUsing = OnRep_PickedUp, VisibleInstanceOnly, BlueprintReadOnly, Category = "Pickup")
	bool bPickedUp = false;

	// 서버에서 호출됨 (REPlayerCharacter::ServerInteract 경유)
	virtual void Interact_Implementation(AActor* Interactor) override;

	// 자식 클래스에서 픽업 조건 확장 가능
	virtual bool CanPickup(AActor* Interactor) const;

	UFUNCTION()
	void OnRep_PickedUp();

	// 소유 클라이언트에서 인벤토리에 추가 (SetOwner + Client RPC 패턴)
	UFUNCTION(Client, Reliable)
	void ClientAddToInventory(AActor* PickedBy);

	// 픽업 연출 — 모든 머신에서 실행
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnPickedUp();

	// SFX/VFX는 BP에서 구현
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickedUpEffects();
	
private:
	void ApplyPickedUpVisual();
};
