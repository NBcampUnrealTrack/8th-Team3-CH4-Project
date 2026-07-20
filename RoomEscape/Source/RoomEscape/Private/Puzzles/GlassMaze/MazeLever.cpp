// Fill out your copyright notice in the Description page of Project Settings.


#include "Puzzles/GlassMaze/MazeLever.h"

#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Puzzles/GlassMaze/GlassMazeManager.h"

AMazeLever::AMazeLever()
{
	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	LeverMesh->SetupAttachment(SceneRoot);
	LeverMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AMazeLever::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMazeLever, bIsOn);
}

void AMazeLever::BeginPlay()
{
	Super::BeginPlay();

	if (AGlassMazeManager* MazeManager = Cast<AGlassMazeManager>(GetPuzzleManager()))
	{
		MazeManager->RegisterLever(this);
	}
}

void AMazeLever::ResetLeverState()
{
	if (HasAuthority() == false || bIsOn == false)
	{
		return;
	}

	bIsOn = false;
	OnRep_IsOn();	// 리슨 서버 호스트
}

void AMazeLever::HandleInteract(AActor* Interactor)
{
	Super::HandleInteract(Interactor);

	AGlassMazeManager* MazeManager = Cast<AGlassMazeManager>(GetPuzzleManager());
	if (!MazeManager || LeverID.IsNone())
	{
		return;
	}

	bIsOn = !bIsOn;
	OnRep_IsOn();	// 리슨 서버 호스트
	MazeManager->SetWallsOpenByLever(LeverID, bIsOn);
}

void AMazeLever::OnRep_IsOn()
{
	BP_OnLeverToggled(bIsOn);
}
