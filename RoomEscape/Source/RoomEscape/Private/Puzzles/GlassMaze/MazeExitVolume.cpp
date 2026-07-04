// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/GlassMaze/MazeExitVolume.h"

#include "Components/BoxComponent.h"
#include "Character/REPlayerCharacter.h"
#include "Puzzles/GlassMaze/GlassMazeManager.h"

AMazeExitVolume::AMazeExitVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMazeExitVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMazeExitVolume::OnTriggerBeginOverlap);
	}
}

void AMazeExitVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AREPlayerCharacter>(OtherActor) && Manager)
	{
		Manager->HandleExitReached(OtherActor);
	}
}
