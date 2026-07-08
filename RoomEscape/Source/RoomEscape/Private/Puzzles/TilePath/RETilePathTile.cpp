#include "Puzzles/TilePath/RETilePathTile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/TilePath/RETilePathManager.h"

ARETilePathTile::ARETilePathTile()
{
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(SceneRoot);
	TileMesh->SetCollisionProfileName(TEXT("BlockAll"));

	StepCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("StepCollision"));
	StepCollision->SetupAttachment(SceneRoot);
	StepCollision->SetBoxExtent(StepCollisionExtent);
	StepCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StepCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	StepCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

void ARETilePathTile::BeginPlay()
{
	Super::BeginPlay();

	StepCollision->SetBoxExtent(StepCollisionExtent);
	StepCollision->OnComponentBeginOverlap.RemoveAll(this);
	StepCollision->OnComponentBeginOverlap.AddDynamic(this, &ARETilePathTile::OnStepCollisionBeginOverlap);

	if (ARETilePathManager* Manager = GetTilePathManager())
	{
		Manager->RegisterTile(this);
	}

	ApplyTileStateVisual();
	ApplyWalkableCollision();
}

void ARETilePathTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TileCoordinate);
	DOREPLIFETIME(ThisClass, TileState);
	DOREPLIFETIME(ThisClass, bWalkable);
}

void ARETilePathTile::SetTilePathManager(ARETilePathManager* InManager)
{
	SetPuzzleManager(InManager);

	if (IsValid(InManager) == true)
	{
		InManager->RegisterTile(this);
	}
}

void ARETilePathTile::SetTileCoordinate(FIntPoint InTileCoordinate)
{
	TileCoordinate = InTileCoordinate;
	OnRep_TileCoordinate();
}

FIntPoint ARETilePathTile::GetTileCoordinate() const
{
	return TileCoordinate;
}

ARETilePathManager* ARETilePathTile::GetTilePathManager() const
{
	return Cast<ARETilePathManager>(PuzzleManager);
}

ERETilePathTileState ARETilePathTile::GetTileState() const
{
	return TileState;
}

bool ARETilePathTile::IsWalkable() const
{
	return bWalkable;
}

void ARETilePathTile::ApplyServerTileState(ERETilePathTileState NewTileState, bool bNewWalkable)
{
	if (HasAuthority() == false)
	{
		return;
	}

	const bool bStateChanged = TileState != NewTileState;
	const bool bWalkableChanged = bWalkable != bNewWalkable;
	TileState = NewTileState;
	bWalkable = bNewWalkable;

	if (bStateChanged == true)
	{
		OnRep_TileState();
	}

	if (bWalkableChanged == true)
	{
		OnRep_Walkable();
	}
}

void ARETilePathTile::OnStepCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Pawn) == false || IsValid(Manager) == false || IsPawnInStepAcceptanceArea(Pawn) == false)
	{
		return;
	}

	Manager->HandleTileStepped(this, Pawn);
}

void ARETilePathTile::OnRep_TileCoordinate()
{
}

void ARETilePathTile::OnRep_TileState()
{
	ApplyTileStateVisual();
	ReceiveTileStateChanged(TileState, bWalkable);
}

void ARETilePathTile::OnRep_Walkable()
{
	ApplyWalkableCollision();
	ReceiveTileStateChanged(TileState, bWalkable);
}

void ARETilePathTile::ApplyTileStateVisual()
{
	if (IsValid(TileMesh) == false)
	{
		return;
	}

	if (UMaterialInterface* Material = GetMaterialForState(TileState))
	{
		TileMesh->SetMaterial(0, Material);
	}

	if (bMoveMeshByState == true)
	{
		const bool bActiveState = TileState == ERETilePathTileState::Available || TileState == ERETilePathTileState::Current || TileState == ERETilePathTileState::Passed || TileState == ERETilePathTileState::Solved;
		const float ZOffset = bActiveState == true ? ActiveMeshZOffset : LockedMeshZOffset;
		TileMesh->SetRelativeLocation(FVector(0.0, 0.0, ZOffset));
	}
}

void ARETilePathTile::ApplyWalkableCollision()
{
	if (IsValid(TileMesh) == false)
	{
		return;
	}

	TileMesh->SetCollisionEnabled(bWalkable == true ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}


bool ARETilePathTile::IsPawnInStepAcceptanceArea(APawn* Pawn) const
{
	if (IsValid(Pawn) == false)
	{
		return false;
	}

	const FVector LocalPawnLocation = GetActorTransform().InverseTransformPosition(Pawn->GetActorLocation());
	if (FMath::Abs(LocalPawnLocation.X) > StepAcceptanceHalfExtent.X || FMath::Abs(LocalPawnLocation.Y) > StepAcceptanceHalfExtent.Y)
	{
		return false;
	}

	return LocalPawnLocation.Z >= MinimumStepLocalZ && LocalPawnLocation.Z <= MaximumStepLocalZ;
}

UMaterialInterface* ARETilePathTile::GetMaterialForState(ERETilePathTileState InTileState) const
{
	switch (InTileState)
	{
	case ERETilePathTileState::Locked:
		return LockedMaterial;
	case ERETilePathTileState::Available:
		return AvailableMaterial;
	case ERETilePathTileState::Current:
		return CurrentMaterial ? CurrentMaterial : AvailableMaterial;
	case ERETilePathTileState::Passed:
		return PassedMaterial ? PassedMaterial : AvailableMaterial;
	case ERETilePathTileState::Failed:
		return FailedMaterial;
	case ERETilePathTileState::Solved:
		return SolvedMaterial ? SolvedMaterial : PassedMaterial;
	default:
		return nullptr;
	}
}
