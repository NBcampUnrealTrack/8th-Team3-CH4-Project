#include "Puzzles/BombDefusal/Components/REBombButtonComponent.h"

#include "Materials/MaterialInterface.h"

UREBombButtonComponent::UREBombButtonComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(TEXT("BlockAll"));
}

void UREBombButtonComponent::BeginPlay()
{
	Super::BeginPlay();
	CaptureReleasedTransform();
}

FName UREBombButtonComponent::GetButtonId() const
{
	return ButtonId;
}

bool UREBombButtonComponent::IsRuntimePressed() const
{
	return bRuntimePressed;
}

void UREBombButtonComponent::ApplyDefinition(const FREBombButtonDefinition& ButtonDefinition)
{
	if (ButtonDefinition.ButtonMesh != nullptr)
	{
		SetStaticMesh(ButtonDefinition.ButtonMesh.Get());
	}

	if (ButtonDefinition.ReleasedMaterial != nullptr)
	{
		ReleasedMaterial = ButtonDefinition.ReleasedMaterial.Get();
	}
	else if (ReleasedMaterial == nullptr)
	{
		ReleasedMaterial = GetMaterial(0);
	}
	PressedMaterial = ButtonDefinition.PressedMaterial;

	if (ReleasedMaterial != nullptr && bRuntimePressed == false)
	{
		SetMaterial(0, ReleasedMaterial.Get());
	}

	ReceiveButtonDefinitionApplied(ButtonDefinition);
}

void UREBombButtonComponent::ApplyPressedState(bool bNewPressed)
{
	CaptureReleasedTransform();

	const bool bStateChanged = bHasAppliedRuntimeState == false || bRuntimePressed != bNewPressed;
	bRuntimePressed = bNewPressed;
	bHasAppliedRuntimeState = true;
	RefreshVisualState();

	if (bStateChanged == true)
	{
		ReceiveButtonPressedChanged(bRuntimePressed);
	}
}

void UREBombButtonComponent::CaptureReleasedTransform()
{
	if (bReleasedStateCaptured == true)
	{
		return;
	}

	ReleasedRelativeTransform = GetRelativeTransform();
	if (ReleasedMaterial == nullptr)
	{
		ReleasedMaterial = GetMaterial(0);
	}
	bReleasedStateCaptured = true;
}

void UREBombButtonComponent::RefreshVisualState()
{
	if (bReleasedStateCaptured == false)
	{
		return;
	}

	if (bApplyPressedTransform == true && bRuntimePressed == true)
	{
		// Apply the offset in the button's authored local space, so a rotated
		// button still travels along its own press axis.
		SetRelativeTransform(PressedTransformOffset * ReleasedRelativeTransform);
	}
	else
	{
		SetRelativeTransform(ReleasedRelativeTransform);
	}

	UMaterialInterface* DesiredMaterial = bRuntimePressed == true && PressedMaterial != nullptr
		? PressedMaterial.Get()
		: ReleasedMaterial.Get();
	if (DesiredMaterial != nullptr)
	{
		SetMaterial(0, DesiredMaterial);
	}
}
