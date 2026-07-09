#include "Puzzles/LockAndGlow/REGlowPaintClueActor.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "Character/REPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/LockAndGlow/RELockAndGlowClueManager.h"
#include "UObject/ConstructorHelpers.h"

AREGlowPaintClueActor::AREGlowPaintClueActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

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
		if (IsValid(CluePlaneComponent) == true)
		{
			CluePlaneComponent->SetVisibility(true, true);
			CluePlaneComponent->SetHiddenInGame(false, true);
		}
	}
}

void AREGlowPaintClueActor::BeginPlay()
{
	if (HasAuthority() == true)
	{
		SetReplicates(true);
		SetReplicateMovement(false);
	}

	Super::BeginPlay();

	ApplyClueMaterial();
	ApplyClueVisibility(false);
	SetActorTickInterval(FMath::Max(VisibilityRefreshInterval, 0.02f));

	if (ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager())
	{
		LockManager->RegisterGlowClue(this);
	}

	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::ServerRelevantFlashlights)
	{
		if (HasAuthority() == true)
		{
			RefreshClueVisibility();
		}
		else
		{
			ApplyClueVisibility(bReplicatedClueVisible);
		}
		return;
	}

	RefreshClueVisibility();
}

void AREGlowPaintClueActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::ServerRelevantFlashlights && HasAuthority() == false)
	{
		return;
	}

	RefreshClueVisibility();
}

void AREGlowPaintClueActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bReplicatedClueVisible);
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
	VisibleEmissiveMaterial = NewMaterial;
	ApplyClueMaterial();
}

void AREGlowPaintClueActor::RefreshClueVisibility()
{
	const bool bNewVisible = ShouldBeVisible();

	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::ServerRelevantFlashlights)
	{
		if (HasAuthority() == true)
		{
			SetAuthoritativeClueVisibility(bNewVisible);
		}
		else
		{
			ApplyClueVisibility(bReplicatedClueVisible);
		}
		return;
	}

	ApplyClueVisibility(bNewVisible);
}

void AREGlowPaintClueActor::OnRep_ReplicatedClueVisible()
{
	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::ServerRelevantFlashlights)
	{
		ApplyClueVisibility(bReplicatedClueVisible);
	}
}

bool AREGlowPaintClueActor::ShouldBeVisible() const
{
	if (ShouldBeVisibleByPuzzleState() == false)
	{
		return false;
	}

	if (bRequireRelevantPlayerToReveal == true && HasRelevantViewer() == false)
	{
		return false;
	}

	if (bVisibleOnlyWhenFlashlightOff == false)
	{
		return true;
	}

	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::ServerRelevantFlashlights)
	{
		return IsAnyRelevantFlashlightOn() == false;
	}

	return IsLocalFlashlightOn() == false;
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

bool AREGlowPaintClueActor::HasRelevantViewer() const
{
	if (VisibilityAuthority == EREGlowClueVisibilityAuthority::LocalViewingPlayer)
	{
		return HasLocalRelevantViewer();
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false)
		{
			continue;
		}

		const APawn* Pawn = PlayerController->GetPawn();
		if (IsFlashlightRelevant(Pawn) == true)
		{
			return true;
		}
	}

	return false;
}

bool AREGlowPaintClueActor::HasLocalRelevantViewer() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
		{
			continue;
		}

		return IsFlashlightRelevant(PlayerController->GetPawn());
	}

	return false;
}

bool AREGlowPaintClueActor::IsLocalFlashlightOn() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
		{
			continue;
		}

		return IsPawnFlashlightOn(PlayerController->GetPawn());
	}

	return false;
}

bool AREGlowPaintClueActor::IsAnyRelevantFlashlightOn() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false)
		{
			continue;
		}

		const APawn* Pawn = PlayerController->GetPawn();
		if (IsPawnFlashlightOn(Pawn) == true && IsFlashlightRelevant(Pawn) == true)
		{
			return true;
		}
	}

	return false;
}

bool AREGlowPaintClueActor::IsPawnFlashlightOn(const APawn* Pawn) const
{
	if (IsValid(Pawn) == false)
	{
		return false;
	}

	if (const AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(Pawn))
	{
		return PlayerCharacter->IsFlashlightOn();
	}

	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Pawn))
	{
		const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();
		return IsValid(AbilitySystemComponent) == true && AbilitySystemComponent->HasMatchingGameplayTag(RETag::State::Flashlight::On) == true;
	}

	return false;
}

bool AREGlowPaintClueActor::IsFlashlightRelevant(const APawn* Pawn) const
{
	if (IsValid(Pawn) == false)
	{
		return false;
	}

	if (FlashlightInfluenceRadius > 0.0f)
	{
		const float RadiusSquared = FMath::Square(FlashlightInfluenceRadius);
		if (FVector::DistSquared(Pawn->GetActorLocation(), GetActorLocation()) > RadiusSquared)
		{
			return false;
		}
	}

	if (bRequireLineOfSightToInfluencingFlashlight == true)
	{
		UWorld* World = GetWorld();
		if (IsValid(World) == false)
		{
			return false;
		}

		FVector ViewLocation = Pawn->GetActorLocation();
		FRotator ViewRotation = FRotator::ZeroRotator;
		Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(REGlowClueFlashlightLineOfSight), false);
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(Pawn);

		FHitResult HitResult;
		if (World->LineTraceSingleByChannel(HitResult, GetActorLocation(), ViewLocation, ECC_Visibility, QueryParams) == true)
		{
			return false;
		}
	}

	return true;
}

void AREGlowPaintClueActor::SetAuthoritativeClueVisibility(bool bNewVisible)
{
	const bool bChanged = bReplicatedClueVisible != bNewVisible;
	bReplicatedClueVisible = bNewVisible;
	ApplyClueVisibility(bReplicatedClueVisible);

	if (bChanged == true)
	{
		ForceNetUpdate();
	}
}

void AREGlowPaintClueActor::ApplyClueMaterial()
{
	if (IsValid(CluePlaneComponent) == true && IsValid(VisibleEmissiveMaterial) == true)
	{
		CluePlaneComponent->SetMaterial(0, VisibleEmissiveMaterial);
	}
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
		CluePlaneComponent->SetCollisionEnabled(bCurrentlyVisible == true && bEnableCollisionWhenVisible == true ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	ReceiveClueVisibilityChanged(bCurrentlyVisible);
}
