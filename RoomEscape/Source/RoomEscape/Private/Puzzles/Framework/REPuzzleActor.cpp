#include "Puzzles/Framework/REPuzzleActor.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"

AREPuzzleActor::AREPuzzleActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AREPuzzleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PuzzleManager);
}

void AREPuzzleActor::SetPuzzleManager(AREPuzzleManager* InPuzzleManager)
{
	PuzzleManager = InPuzzleManager;
}

AREPuzzleManager* AREPuzzleActor::GetPuzzleManager() const
{
	return PuzzleManager;
}
