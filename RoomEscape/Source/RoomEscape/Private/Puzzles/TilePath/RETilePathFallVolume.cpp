#include "Puzzles/TilePath/RETilePathFallVolume.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Puzzles/TilePath/RETilePathManager.h"

ARETilePathFallVolume::ARETilePathFallVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	FallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FallCollision"));
	FallCollision->SetupAttachment(SceneRoot);
	FallCollision->SetBoxExtent(FVector(700.0, 700.0, 100.0));
	FallCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FallCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	FallCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FallCollision->SetGenerateOverlapEvents(true);
}

void ARETilePathFallVolume::BeginPlay()
{
	Super::BeginPlay();

	FallCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FallCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	FallCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FallCollision->SetGenerateOverlapEvents(true);
	FallCollision->OnComponentBeginOverlap.RemoveAll(this);
	FallCollision->OnComponentEndOverlap.RemoveAll(this);
	FallCollision->OnComponentBeginOverlap.AddDynamic(this, &ARETilePathFallVolume::OnFallCollisionBeginOverlap);
	FallCollision->OnComponentEndOverlap.AddDynamic(this, &ARETilePathFallVolume::OnFallCollisionEndOverlap);
	ResolvePuzzleManagerIfNeeded();
}


void ARETilePathFallVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == false)
	{
		return;
	}

	CleanupProcessedPawns();
	for (auto It = OverlappingPawns.CreateIterator(); It; ++It)
	{
		APawn* Pawn = It->Get();
		if (IsValid(Pawn) == false || FallCollision->IsOverlappingActor(Pawn) == false)
		{
			It.RemoveCurrent();
			continue;
		}

		ProcessFallenPawn(Pawn);
	}
}

void ARETilePathFallVolume::SetTilePathManager(ARETilePathManager* InManager)
{
	SetPuzzleManager(InManager);
}

ARETilePathManager* ARETilePathFallVolume::GetTilePathManager() const
{
	return Cast<ARETilePathManager>(PuzzleManager);
}

void ARETilePathFallVolume::ProcessFallenPawn(APawn* FallingPawn)
{
	if (HasAuthority() == false || IsValid(FallingPawn) == false || CanProcessFallingPawn(FallingPawn) == false)
	{
		return;
	}

	CleanupProcessedPawns();
	LastProcessedTimes.FindOrAdd(TWeakObjectPtr<APawn>(FallingPawn)) = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	ResolvePuzzleManagerIfNeeded();

	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Manager) == true)
	{
		Manager->HandleWalkerFell(FallingPawn);
	}
}

void ARETilePathFallVolume::ResolvePuzzleManagerIfNeeded()
{
	if (IsValid(PuzzleManager) == true)
	{
		return;
	}

	ARETilePathManager* ClosestManager = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	const FVector MyLocation = GetActorLocation();

	for (TActorIterator<ARETilePathManager> It(GetWorld()); It; ++It)
	{
		ARETilePathManager* Candidate = *It;
		if (IsValid(Candidate) == false)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestManager = Candidate;
		}
	}

	if (IsValid(ClosestManager) == true)
	{
		SetPuzzleManager(ClosestManager);
	}
}


bool ARETilePathFallVolume::CanProcessFallingPawn(APawn* FallingPawn) const
{
	if (IsValid(FallingPawn) == false || IsValid(FallCollision) == false)
	{
		return false;
	}

	const TWeakObjectPtr<APawn> PawnKey(FallingPawn);
	const double CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (const double* LastProcessedTime = LastProcessedTimes.Find(PawnKey))
	{
		if (CurrentTime - *LastProcessedTime < MinimumReprocessIntervalSeconds)
		{
			return false;
		}
	}

	if (bRequirePawnBelowVolumeTop == true)
	{
		const float VolumeTopZ = FallCollision->Bounds.Origin.Z + FallCollision->Bounds.BoxExtent.Z;
		if (FallingPawn->GetActorLocation().Z > VolumeTopZ - VolumeTopTolerance)
		{
			return false;
		}
	}

	if (bRequireDownwardVelocity == true && FallingPawn->GetVelocity().Z > RequiredDownwardVelocityZ)
	{
		return false;
	}

	return true;
}

void ARETilePathFallVolume::CleanupProcessedPawns()
{
	const double CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	for (auto It = LastProcessedTimes.CreateIterator(); It; ++It)
	{
		const bool bInvalidPawn = It.Key().IsValid() == false;
		const bool bExpired = CurrentTime - It.Value() > MinimumReprocessIntervalSeconds;
		if (bInvalidPawn == true || bExpired == true)
		{
			It.RemoveCurrent();
		}
	}
}

void ARETilePathFallVolume::OnFallCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* FallingPawn = Cast<APawn>(OtherActor);
	if (IsValid(FallingPawn) == false)
	{
		return;
	}

	const TWeakObjectPtr<APawn> PawnKey(FallingPawn);
	if (OverlappingPawns.Contains(PawnKey) == true)
	{
		return;
	}

	OverlappingPawns.Add(PawnKey);
	ProcessFallenPawn(FallingPawn);
}

void ARETilePathFallVolume::OnFallCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* LeavingPawn = Cast<APawn>(OtherActor);
	if (IsValid(LeavingPawn) == true)
	{
		OverlappingPawns.Remove(TWeakObjectPtr<APawn>(LeavingPawn));
	}
}
