// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/REPickupItemActor.h"

#include "AbilitySystem/NativeGameplayTags.h"
#include "Components/REInventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/RENotifySubsystem.h"
#include "Item/ItemDataAsset.h"
#include "Net/UnrealNetwork.h"


class URENotifySubsystem;

AREPickupItemActor::AREPickupItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AREPickupItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AREPickupItemActor, bPickedUp);
}

void AREPickupItemActor::Interact_Implementation(AActor* Interactor)
{
	if (HasAuthority() == false || CanPickup(Interactor) == false)
	{
		return;
	}

	bPickedUp = true;
	// 리슨 서버(호스트)에서는 OnRep이 호출되지 않으므로 직접 적용
	ApplyPickedUpVisual();

	// 인벤토리는 로컬(비복제) 컴포넌트 — 소유 클라이언트에서 추가
	SetOwner(Interactor);
	ClientAddToInventory(Interactor);

	MulticastOnPickedUp();

	if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Item::PickedUp,
				FString::Printf(TEXT("%s이(가) %s을(를) 획득했습니다"),
						*GetNameSafe(Interactor), *GetNameSafe(ItemData)));
	}
}

bool AREPickupItemActor::CanPickup(AActor* Interactor) const
{
	if (bPickedUp == true || IsValid(Interactor) == false)
	{
		return false;
	}

	if (IsValid(ItemData) == false)
	{
		// 배치 인스턴스에서 ItemData 미지정 — 퍼즐 매니저 미지정 사고와 같은 유형이므로 로그로 노출
		UE_LOG(LogTemp, Warning, TEXT("[%s] ItemData=None — 배치 인스턴스에서 지정 필요"), *GetName());
		return false;
	}

	return true;
}

void AREPickupItemActor::OnRep_PickedUp()
{
	if (bPickedUp == true)
	{
		ApplyPickedUpVisual();
	}
}

void AREPickupItemActor::ClientAddToInventory_Implementation(AActor* PickedBy)
{
	UREInventoryComponent* Inventory =
			IsValid(PickedBy) ? PickedBy->FindComponentByClass<UREInventoryComponent>() : nullptr;
	if (Inventory == nullptr || IsValid(ItemData) == false)
	{
		return;
	}

	if (Inventory->AddItemToInventory(ItemData->GetPrimaryAssetId()) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 인벤토리 추가 실패 (가득 참?) — 아이템은 이미 월드에서 제거됨"), *GetName());
	}
}

void AREPickupItemActor::MulticastOnPickedUp_Implementation()
{
	OnPickedUpEffects();
}

void AREPickupItemActor::ApplyPickedUpVisual()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

