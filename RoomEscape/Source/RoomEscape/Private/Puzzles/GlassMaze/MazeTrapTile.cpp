// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/GlassMaze/MazeTrapTile.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/REPlayerCharacter.h"
#include "Puzzles/GlassMaze/GlassMazeManager.h"

AMazeTrapTile::AMazeTrapTile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	SetRootComponent(TileMesh);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(TileMesh);
	TriggerBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMazeTrapTile::BeginPlay()
{
	Super::BeginPlay();

	// 함정 판정은 서버에서만 (규약)
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMazeTrapTile::OnTriggerBeginOverlap);
	}
}

void AMazeTrapTile::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AREPlayerCharacter* Victim = Cast<AREPlayerCharacter>(OtherActor);
	if (Victim && Manager)
	{
		Manager->HandleTrapTriggered(this, Victim);
	}
}

void AMazeTrapTile::Multicast_OnTriggered_Implementation()
{
	BP_OnTriggered();
}
