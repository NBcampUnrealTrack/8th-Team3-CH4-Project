#include "Puzzles/BombDefusal/Components/REBombWireComponent.h"

#include "Materials/MaterialInterface.h"

UREBombWireComponent::UREBombWireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(TEXT("BlockAll"));
}

void UREBombWireComponent::BeginPlay()
{
	Super::BeginPlay();
	CaptureInitialState();
}

int32 UREBombWireComponent::GetWireIndex() const
{
	return WireIndex;
}

bool UREBombWireComponent::IsRuntimeCut() const
{
	return bRuntimeCut;
}

void UREBombWireComponent::ApplyDefinition(const FREBombWireDefinition& WireDefinition)
{
	if (WireDefinition.WireMesh != nullptr)
	{
		SetStaticMesh(WireDefinition.WireMesh.Get());
	}

	if (WireDefinition.WireMaterial != nullptr)
	{
		SetMaterial(0, WireDefinition.WireMaterial.Get());
	}

	bHideWhenCut = WireDefinition.bHideVisualWhenCut;
	ReceiveWireDefinitionApplied(WireDefinition);
}

void UREBombWireComponent::ApplyCutState(bool bNewCut)
{
	CaptureInitialState();

	const bool bStateChanged = bHasAppliedRuntimeState == false || bRuntimeCut != bNewCut;
	bRuntimeCut = bNewCut;
	bHasAppliedRuntimeState = true;
	RefreshVisualState();

	if (bStateChanged == true)
	{
		ReceiveWireCutChanged(bRuntimeCut);
	}
}

void UREBombWireComponent::CaptureInitialState()
{
	if (bInitialStateCaptured == true)
	{
		return;
	}

	InitialCollisionEnabled = GetCollisionEnabled();
	bInitialVisibility = IsVisible();
	bInitialStateCaptured = true;
}

void UREBombWireComponent::RefreshVisualState()
{
	if (bInitialStateCaptured == false)
	{
		return;
	}

	const bool bShouldBeVisible = bInitialVisibility
		&& (bRuntimeCut == false || bHideWhenCut == false);

	// Each wire is an independent puzzle element. Never propagate visibility
	// to attached child components: a nested Blueprint component hierarchy must
	// not make cutting one wire hide every wire below it.
	SetVisibility(bShouldBeVisible, false);

	if (bRuntimeCut == true && bDisableCollisionWhenCut == true)
	{
		SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		SetCollisionEnabled(InitialCollisionEnabled);
	}
}
