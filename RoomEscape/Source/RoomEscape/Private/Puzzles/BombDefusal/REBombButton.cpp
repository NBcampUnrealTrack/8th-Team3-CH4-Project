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

	InteractionCollision->SetBoxExtent(FVector(90.0, 90.0, 90.0));
	InteractionPromptText = FText::FromString(TEXT("버튼 조작"));
	InteractionPromptRelativeLocation = FVector(0.0, 0.0, 100.0);

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

APlayerState* AREBombButton::GetPressingPlayerState() const
{
	return PressingPlayerState;
}

void AREBombButton::ApplyServerPressedState(bool bNewPressed, APlayerState* InPressingPlayerState)
{
	if (HasAuthority() == false)
	{
		return;
	}

	bPressed = bNewPressed;
	PressingPlayerState = bNewPressed == true ? InPressingPlayerState : nullptr;
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

	BombManager->SubmitButtonToggle(this, Interactor);
}

void AREBombButton::OnRep_ButtonPressed()
{
	ReceiveButtonPressedChanged(bPressed);
}
