// Fill out your copyright notice in the Description page of Project Settings.


#include "Progression/REProgressionDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AREProgressionDoor::AREProgressionDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AREProgressionDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bIsOpen);
}

// Called when the game starts or when spawned
void AREProgressionDoor::BeginPlay()
{
	Super::BeginPlay();
	
	ApplyDoorState();
}

void AREProgressionDoor::SetOpen(bool bNewIsOpen)
{
	if (HasAuthority() == false || bIsOpen == bNewIsOpen)
	{
		return;
	}

	bIsOpen = bNewIsOpen;
	OnRep_IsOpen();
}

bool AREProgressionDoor::IsOpen() const
{
	return bIsOpen;
}

void AREProgressionDoor::OnRep_IsOpen()
{
	ApplyDoorState();
	OnDoorStateChanged.Broadcast(bIsOpen);
	ReceiveDoorStateChanged(bIsOpen);
}

void AREProgressionDoor::ApplyDoorState()
{
	DoorMesh->SetCollisionEnabled(bIsOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

	if (bHideMeshWhenOpen == true)
	{
		DoorMesh->SetVisibility(bIsOpen == false, true);
	}
}