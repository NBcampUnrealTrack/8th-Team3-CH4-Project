#include "Puzzles/LockAndGlow/REGlowPaintClueActor.h"

#include "Character/REPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Puzzles/LockAndGlow/RELockAndGlowClueManager.h"
#include "UObject/ConstructorHelpers.h"

namespace REGlowRevealMaterialParameters
{
	constexpr int32 MaxSupportedFlashlights = 2;

	const FName RevealEnabled(TEXT("GlowReveal_Enabled"));

	const FName LightEnabled[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_Enabled"),
		TEXT("GlowReveal_Light1_Enabled")
	};

	const FName LightPosition[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_Position"),
		TEXT("GlowReveal_Light1_Position")
	};

	const FName LightDirection[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_Direction"),
		TEXT("GlowReveal_Light1_Direction")
	};

	const FName LightRange[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_Range"),
		TEXT("GlowReveal_Light1_Range")
	};

	const FName LightInnerConeCos[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_InnerConeCos"),
		TEXT("GlowReveal_Light1_InnerConeCos")
	};

	const FName LightOuterConeCos[MaxSupportedFlashlights] =
	{
		TEXT("GlowReveal_Light0_OuterConeCos"),
		TEXT("GlowReveal_Light1_OuterConeCos")
	};
}

AREGlowPaintClueActor::AREGlowPaintClueActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CluePlaneComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CluePlane"));
	CluePlaneComponent->SetupAttachment(SceneRoot);
	CluePlaneComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CluePlaneComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CluePlaneComponent->SetGenerateOverlapEvents(false);
	CluePlaneComponent->SetCanEverAffectNavigation(false);
	CluePlaneComponent->SetCastShadow(false);
	CluePlaneComponent->SetVisibility(false, true);
	CluePlaneComponent->SetHiddenInGame(true, true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (DefaultPlaneMesh.Succeeded() == true)
	{
		CluePlaneComponent->SetStaticMesh(DefaultPlaneMesh.Object);
	}
}

void AREGlowPaintClueActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyClueMaterial();

	if (UWorld* World = GetWorld(); IsValid(World) == true && World->IsGameWorld() == false)
	{
		CluePlaneComponent->SetVisibility(true, true);
		CluePlaneComponent->SetHiddenInGame(false, true);
	}
}

void AREGlowPaintClueActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickInterval(FMath::Clamp(VisibilityRefreshInterval, 0.02f, 0.25f));
	EnsureDynamicRevealMaterial();
	ApplyClueVisibility(false);

	if (ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager())
	{
		LockManager->RegisterGlowClue(this);
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		SetActorTickEnabled(false);
		return;
	}

	RefreshClueVisibility();
}

void AREGlowPaintClueActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshClueVisibility();
}

void AREGlowPaintClueActor::SetLockAndGlowManager(ARELockAndGlowClueManager* InManager)
{
	SetPuzzleManager(InManager);
}

ARELockAndGlowClueManager* AREGlowPaintClueActor::GetLockAndGlowManager() const
{
	return Cast<ARELockAndGlowClueManager>(PuzzleManager);
}

bool AREGlowPaintClueActor::IsClueVisible() const
{
	return bCurrentlyVisible;
}

UStaticMeshComponent* AREGlowPaintClueActor::GetCluePlaneComponent() const
{
	return CluePlaneComponent;
}

void AREGlowPaintClueActor::SetVisibleEmissiveMaterial(UMaterialInterface* NewMaterial)
{
	if (VisibleEmissiveMaterial == NewMaterial)
	{
		return;
	}

	VisibleEmissiveMaterial = NewMaterial;
	DynamicRevealMaterial = nullptr;
	ApplyClueMaterial();

	if (GetWorld() != nullptr && GetWorld()->IsGameWorld() == true)
	{
		EnsureDynamicRevealMaterial();
		RefreshClueVisibility();
	}
}

void AREGlowPaintClueActor::RefreshClueVisibility()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const bool bPuzzleAllowsReveal = ShouldBeVisibleByPuzzleState();
	TArray<FFlashlightRevealData> AffectingFlashlights;

	if (bPuzzleAllowsReveal == true)
	{
		CollectAffectingFlashlights(AffectingFlashlights);
	}

	UpdateMaterialRevealParameters(bPuzzleAllowsReveal, AffectingFlashlights);
	ApplyClueVisibility(bPuzzleAllowsReveal == true && AffectingFlashlights.IsEmpty() == false);
}


void AREGlowPaintClueActor::HandlePuzzleManagerChanged()
{
	Super::HandlePuzzleManagerChanged();

	if (HasActorBegunPlay() == false)
	{
		return;
	}

	if (ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager())
	{
		LockManager->RegisterGlowClue(this);
	}

	RefreshClueVisibility();
}

bool AREGlowPaintClueActor::ShouldBeVisibleByPuzzleState() const
{
	const ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager();
	if (IsValid(LockManager) == false)
	{
		return true;
	}

	if (LockManager->IsActive() == false && LockManager->IsSolved() == false)
	{
		return false;
	}

	if (bHideAfterPuzzleSolved == true && LockManager->IsSolved() == true)
	{
		return false;
	}

	return true;
}

void AREGlowPaintClueActor::CollectAffectingFlashlights(TArray<FFlashlightRevealData>& OutFlashlights) const
{
	OutFlashlights.Reset();

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	for (TActorIterator<AREPlayerCharacter> Iterator(World); Iterator; ++Iterator)
	{
		FFlashlightRevealData FlashlightData;
		if (BuildFlashlightRevealData(*Iterator, FlashlightData) == false)
		{
			continue;
		}

		if (IsSourceCharacterWithinRecognitionRange(FlashlightData) == false)
		{
			continue;
		}

		if (DoesFlashlightConeReachClue(FlashlightData) == false)
		{
			continue;
		}

		if (HasUnobstructedFlashlightPath(FlashlightData) == false)
		{
			continue;
		}

		OutFlashlights.Add(MoveTemp(FlashlightData));
	}

	const FVector ClueCenter = CluePlaneComponent->Bounds.Origin;
	OutFlashlights.Sort([ClueCenter](const FFlashlightRevealData& Left, const FFlashlightRevealData& Right)
	{
		return FVector::DistSquared(Left.Position, ClueCenter) < FVector::DistSquared(Right.Position, ClueCenter);
	});

	if (OutFlashlights.Num() > REGlowRevealMaterialParameters::MaxSupportedFlashlights)
	{
		OutFlashlights.SetNum(REGlowRevealMaterialParameters::MaxSupportedFlashlights, EAllowShrinking::No);
	}
}

bool AREGlowPaintClueActor::BuildFlashlightRevealData(AREPlayerCharacter* PlayerCharacter, FFlashlightRevealData& OutData) const
{
	if (IsValid(PlayerCharacter) == false)
	{
		return false;
	}

	float InnerConeAngle = 0.0f;
	float OuterConeAngle = 0.0f;
	if (PlayerCharacter->GetFlashlightBeamData(
		OutData.Position,
		OutData.Direction,
		OutData.Range,
		InnerConeAngle,
		OuterConeAngle) == false)
	{
		return false;
	}

	OutData.SourceCharacter = PlayerCharacter;
	OutData.InnerConeCos = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(InnerConeAngle, 0.0f, 89.0f)));
	OutData.OuterConeCos = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(OuterConeAngle, 0.0f, 89.0f)));
	return OutData.Range > KINDA_SMALL_NUMBER && OutData.Direction.IsNearlyZero() == false;
}

bool AREGlowPaintClueActor::IsSourceCharacterWithinRecognitionRange(
	const FFlashlightRevealData& FlashlightData) const
{
	if (IsValid(CluePlaneComponent) == false)
	{
		return false;
	}

	const AREPlayerCharacter* SourceCharacter = FlashlightData.SourceCharacter.Get();
	if (IsValid(SourceCharacter) == false)
	{
		return false;
	}

	const float RecognitionRange = FMath::Max(FlashlightRecognitionRange, 1.0f);
	const FVector ClueCenter = CluePlaneComponent->Bounds.Origin;
	return FVector::DistSquared(SourceCharacter->GetActorLocation(), ClueCenter)
		<= FMath::Square(RecognitionRange);
}

bool AREGlowPaintClueActor::DoesFlashlightConeReachClue(const FFlashlightRevealData& FlashlightData) const
{
	if (IsValid(CluePlaneComponent) == false)
	{
		return false;
	}

	const FBoxSphereBounds& ClueBounds = CluePlaneComponent->Bounds;
	const FVector ToClueCenter = ClueBounds.Origin - FlashlightData.Position;
	const float CenterDistanceSquared = ToClueCenter.SizeSquared();
	const float CenterDistance = FMath::Sqrt(CenterDistanceSquared);
	const float ClueRadius = ClueBounds.SphereRadius * 0.05f;

	if (CenterDistance <= ClueRadius)
	{
		return true;
	}

	if (CenterDistance - ClueRadius > FlashlightData.Range)
	{
		return false;
	}

	const FVector BeamDirection = FlashlightData.Direction.GetSafeNormal();
	const float AxialDistance = FVector::DotProduct(ToClueCenter, BeamDirection);
	if (AxialDistance < -ClueRadius || AxialDistance > FlashlightData.Range + ClueRadius)
	{
		return false;
	}

	const float RadialDistanceSquared = FMath::Max(CenterDistanceSquared - FMath::Square(AxialDistance), 0.0f);
	const float RadialDistance = FMath::Sqrt(RadialDistanceSquared);
	const float OuterConeAngleRadians = FMath::Acos(FMath::Clamp(FlashlightData.OuterConeCos, -1.0f, 1.0f));
	const float ConeRadiusAtClue = FMath::Max(AxialDistance, 0.0f) * FMath::Tan(OuterConeAngleRadians);

	return RadialDistance <= ConeRadiusAtClue + ClueRadius;
}

bool AREGlowPaintClueActor::HasUnobstructedFlashlightPath(const FFlashlightRevealData& FlashlightData) const
{
	if (bRequireLineOfSightToInfluencingFlashlight == false)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false || IsValid(CluePlaneComponent) == false)
	{
		return false;
	}

	FVector TraceEnd = CluePlaneComponent->Bounds.Origin;
	const FVector TowardFlashlight = (FlashlightData.Position - TraceEnd).GetSafeNormal();
	TraceEnd += TowardFlashlight * FMath::Max(OcclusionTraceEndOffset, 0.0f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(REGlowClueFlashlightOcclusion), false);
	QueryParams.AddIgnoredActor(this);
	if (AREPlayerCharacter* SourceCharacter = FlashlightData.SourceCharacter.Get())
	{
		QueryParams.AddIgnoredActor(SourceCharacter);
	}

	FHitResult HitResult;
	return World->LineTraceSingleByChannel(
		HitResult,
		FlashlightData.Position,
		TraceEnd,
		OcclusionTraceChannel,
		QueryParams) == false;
}

void AREGlowPaintClueActor::ApplyClueMaterial()
{
	if (IsValid(CluePlaneComponent) == false)
	{
		return;
	}

	if (IsValid(DynamicRevealMaterial) == true)
	{
		CluePlaneComponent->SetMaterial(0, DynamicRevealMaterial);
		return;
	}

	if (IsValid(VisibleEmissiveMaterial) == true)
	{
		CluePlaneComponent->SetMaterial(0, VisibleEmissiveMaterial);
	}
}

void AREGlowPaintClueActor::EnsureDynamicRevealMaterial()
{
	if (IsValid(CluePlaneComponent) == false || IsValid(VisibleEmissiveMaterial) == false)
	{
		return;
	}

	if (IsValid(DynamicRevealMaterial) == false)
	{
		DynamicRevealMaterial = UMaterialInstanceDynamic::Create(VisibleEmissiveMaterial, this);
	}

	ApplyClueMaterial();
}

void AREGlowPaintClueActor::UpdateMaterialRevealParameters(
	bool bPuzzleAllowsReveal,
	const TArray<FFlashlightRevealData>& Flashlights)
{
	EnsureDynamicRevealMaterial();
	if (IsValid(DynamicRevealMaterial) == false)
	{
		return;
	}

	using namespace REGlowRevealMaterialParameters;

	DynamicRevealMaterial->SetScalarParameterValue(RevealEnabled, bPuzzleAllowsReveal == true ? 1.0f : 0.0f);

	for (int32 LightIndex = 0; LightIndex < MaxSupportedFlashlights; ++LightIndex)
	{
		SetMaterialFlashlightParameters(LightIndex, Flashlights.IsValidIndex(LightIndex) ? &Flashlights[LightIndex] : nullptr);
	}
}

void AREGlowPaintClueActor::SetMaterialFlashlightParameters(
	int32 LightIndex,
	const FFlashlightRevealData* FlashlightData)
{
	using namespace REGlowRevealMaterialParameters;

	if (IsValid(DynamicRevealMaterial) == false || LightIndex < 0 || LightIndex >= MaxSupportedFlashlights)
	{
		return;
	}

	const bool bEnabled = FlashlightData != nullptr;
	DynamicRevealMaterial->SetScalarParameterValue(LightEnabled[LightIndex], bEnabled == true ? 1.0f : 0.0f);

	if (bEnabled == false)
	{
		DynamicRevealMaterial->SetVectorParameterValue(LightPosition[LightIndex], FLinearColor::Black);
		DynamicRevealMaterial->SetVectorParameterValue(
			LightDirection[LightIndex],
			FLinearColor(FVector::ForwardVector.X, FVector::ForwardVector.Y, FVector::ForwardVector.Z, 0.0f));
		DynamicRevealMaterial->SetScalarParameterValue(LightRange[LightIndex], 1.0f);
		DynamicRevealMaterial->SetScalarParameterValue(LightInnerConeCos[LightIndex], 1.0f);
		DynamicRevealMaterial->SetScalarParameterValue(LightOuterConeCos[LightIndex], 0.9999f);
		return;
	}

	DynamicRevealMaterial->SetVectorParameterValue(
		LightPosition[LightIndex],
		FLinearColor(FlashlightData->Position.X, FlashlightData->Position.Y, FlashlightData->Position.Z, 1.0f));
	DynamicRevealMaterial->SetVectorParameterValue(
		LightDirection[LightIndex],
		FLinearColor(FlashlightData->Direction.X, FlashlightData->Direction.Y, FlashlightData->Direction.Z, 0.0f));
	DynamicRevealMaterial->SetScalarParameterValue(LightRange[LightIndex], FlashlightData->Range);
	DynamicRevealMaterial->SetScalarParameterValue(LightInnerConeCos[LightIndex], FlashlightData->InnerConeCos);
	DynamicRevealMaterial->SetScalarParameterValue(LightOuterConeCos[LightIndex], FlashlightData->OuterConeCos);
}

void AREGlowPaintClueActor::ApplyClueVisibility(bool bNewVisible)
{
	if (bClueVisibilityInitialized == true && bCurrentlyVisible == bNewVisible)
	{
		return;
	}

	bClueVisibilityInitialized = true;
	bCurrentlyVisible = bNewVisible;

	if (IsValid(CluePlaneComponent) == true)
	{
		ApplyClueMaterial();
		CluePlaneComponent->SetVisibility(bCurrentlyVisible, true);
		CluePlaneComponent->SetHiddenInGame(bCurrentlyVisible == false, true);
		CluePlaneComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	ReceiveClueVisibilityChanged(bCurrentlyVisible);
}
