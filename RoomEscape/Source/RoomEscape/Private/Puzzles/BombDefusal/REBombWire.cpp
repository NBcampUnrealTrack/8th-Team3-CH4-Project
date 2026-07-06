#include "Puzzles/BombDefusal/REBombWire.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"

AREBombWire::AREBombWire()
{
	bReplicates = true;

	WireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WireMesh"));
	WireMesh->SetupAttachment(SceneRoot);
	WireMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(30.0, 30.0, 60.0));
}

void AREBombWire::BeginPlay()
{
	Super::BeginPlay();

	if (AREBombDefusalManager* BombManager = GetBombManager())
	{
		BombManager->RegisterWire(this);
	}
	RefreshWireVisuals();
}

void AREBombWire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, WireIndex);
	DOREPLIFETIME(ThisClass, WireColor);
	DOREPLIFETIME(ThisClass, bCut);
}

void AREBombWire::SetBombManager(AREBombDefusalManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREBombDefusalManager* AREBombWire::GetBombManager() const
{
	return Cast<AREBombDefusalManager>(PuzzleManager);
}

int32 AREBombWire::GetWireIndex() const
{
	return WireIndex;
}

EREBombWireColor AREBombWire::GetWireColor() const
{
	return WireColor;
}

bool AREBombWire::IsCut() const
{
	return bCut;
}

void AREBombWire::ApplyServerWireDefinition(const FREBombWireDefinition& WireDefinition)
{
	if (HasAuthority() == false)
	{
		return;
	}

	WireIndex = WireDefinition.WireIndex;
	WireColor = WireDefinition.WireColor;
	OnRep_WireVisualState();
}

void AREBombWire::ApplyServerCutState(bool bNewCut)
{
	if (HasAuthority() == false || bCut == bNewCut)
	{
		return;
	}

	bCut = bNewCut;
	OnRep_WireCut();
}

bool AREBombWire::CanUseElement(AActor* Interactor) const
{
	return Super::CanUseElement(Interactor) == true && bCut == false;
}

void AREBombWire::HandleInteract(AActor* Interactor)
{
	if (AREBombDefusalManager* BombManager = GetBombManager())
	{
		BombManager->SubmitWireCut(this, Interactor);
	}
}

void AREBombWire::OnRep_WireCut()
{
	RefreshWireVisuals();
	ReceiveWireCutChanged(bCut);
}

void AREBombWire::OnRep_WireVisualState()
{
	RefreshWireVisuals();
	ReceiveWireVisualChanged(WireIndex, WireColor);
}

void AREBombWire::RefreshWireVisuals()
{
	if (IsValid(WireMesh) == true)
	{
		WireMesh->SetVisibility(bCut == false, true);
		WireMesh->SetCollisionEnabled(bCut == true ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	}

	if (IsValid(InteractionCollision) == true)
	{
		InteractionCollision->SetCollisionEnabled(bCut == true ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
	}
}
