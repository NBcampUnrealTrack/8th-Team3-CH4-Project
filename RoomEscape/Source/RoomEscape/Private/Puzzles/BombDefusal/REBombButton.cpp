#include "Puzzles/BombDefusal/REBombButton.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"

AREBombButton::AREBombButton()
{
	bReplicates = true;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	ButtonMesh->SetupAttachment(SceneRoot);
	ButtonMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(50.0, 50.0, 50.0));
}

void AREBombButton::BeginPlay()
{
	Super::BeginPlay();

	if (AREBombDefusalManager* BombManager = GetBombManager())
	{
		BombManager->RegisterButton(this);
	}
}

void AREBombButton::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bPressed);
	DOREPLIFETIME(ThisClass, PressingPlayerState);
	DOREPLIFETIME(ThisClass, PressStartServerTimeSeconds);
}

void AREBombButton::SetBombManager(AREBombDefusalManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREBombDefusalManager* AREBombButton::GetBombManager() const
{
	return Cast<AREBombDefusalManager>(PuzzleManager);
}

FName AREBombButton::GetButtonId() const
{
	return ButtonId;
}

bool AREBombButton::IsPressed() const
{
	return bPressed;
}

float AREBombButton::GetPressStartServerTimeSeconds() const
{
	return PressStartServerTimeSeconds;
}

APlayerState* AREBombButton::GetPressingPlayerState() const
{
	return PressingPlayerState;
}

void AREBombButton::ApplyServerPressedState(bool bNewPressed, APlayerState* InPressingPlayerState, float InPressStartServerTimeSeconds)
{
	if (HasAuthority() == false)
	{
		return;
	}

	bPressed = bNewPressed;
	PressingPlayerState = bNewPressed == true ? InPressingPlayerState : nullptr;
	PressStartServerTimeSeconds = bNewPressed == true ? InPressStartServerTimeSeconds : 0.0f;
	OnRep_ButtonPressed();
}

bool AREBombButton::CanUseElement(AActor* Interactor) const
{
	return Super::CanUseElement(Interactor) == true;
}

void AREBombButton::HandleInteract(AActor* Interactor)
{
	AREBombDefusalManager* BombManager = GetBombManager();
	if (IsValid(BombManager) == false)
	{
		return;
	}

	if (bPressed == false)
	{
		BombManager->SubmitButtonPress(this, Interactor);
	}
	else
	{
		BombManager->SubmitButtonRelease(this, Interactor);
	}
}

void AREBombButton::OnRep_ButtonPressed()
{
	ReceiveButtonPressedChanged(bPressed);
}
