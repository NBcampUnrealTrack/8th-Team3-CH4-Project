// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/GlassMaze/GlassWall.h"

#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzles/GlassMaze/GlassMazeManager.h"

AGlassWall::AGlassWall()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	SetRootComponent(WallMesh);
	WallMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AGlassWall::BeginPlay()
{
	Super::BeginPlay();

	if (Manager)
	{
		Manager->RegisterOpenableWall(this);
	}
}

void AGlassWall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGlassWall, bIsOpen);
}

bool AGlassWall::SetOpen(bool bNewOpen)
{
	if (!HasAuthority())
	{
		return false;
	}
	if (bIsOpen == bNewOpen)
	{
		return true;
	}
	if (!bNewOpen && IsBlockedByPawn())
	{
		return false;	// 플레이어가 서 있는 자리는 닫지 않음 (예외 케이스 규약)
	}

	bIsOpen = bNewOpen;
	OnRep_IsOpen();		// 리슨 서버 호스트도 동일 경로 실행
	return true;
}

bool AGlassWall::IsBlockedByPawn() const
{
	// 벽 자리에 폰이 겹쳐 있는지 확인 (약간의 여유 포함)
	const FCollisionShape Shape = FCollisionShape::MakeBox(WallMesh->Bounds.BoxExtent + FVector(20.f));
	return GetWorld()->OverlapAnyTestByObjectType(
		WallMesh->Bounds.Origin, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), Shape);
}

void AGlassWall::OnRep_IsOpen()
{
	ApplyCollision();
	BP_OnOpenStateChanged(bIsOpen);
}

void AGlassWall::ApplyCollision()
{
	// 콜리전은 즉시 전환 (물리로 밀어내지 않음 - 끼임 방지 규약), 시각 연출만 BP 타임라인
	WallMesh->SetCollisionEnabled(bIsOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}
