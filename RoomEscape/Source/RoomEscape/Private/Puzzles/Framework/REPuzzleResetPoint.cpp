#include "Puzzles/Framework/REPuzzleResetPoint.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

AREPuzzleResetPoint::AREPuzzleResetPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(SceneRoot);
	ArrowComponent->ArrowSize = 1.5f;
}

FName AREPuzzleResetPoint::GetCheckpointId() const
{
	return CheckpointId;
}

int32 AREPuzzleResetPoint::GetCheckpointOrder() const
{
	return CheckpointOrder;
}

bool AREPuzzleResetPoint::CanUseAsCheckpoint() const
{
	return bUseAsCheckpoint;
}

FTransform AREPuzzleResetPoint::GetCheckpointTransform() const
{
	return GetActorTransform();
}
