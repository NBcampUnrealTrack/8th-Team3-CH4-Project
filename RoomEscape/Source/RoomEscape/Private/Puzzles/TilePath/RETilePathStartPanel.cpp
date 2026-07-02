#include "Puzzles/TilePath/RETilePathStartPanel.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/TilePath/RETilePathManager.h"
#include "UI/RETilePathWaitingWidget.h"

ARETilePathStartPanel::ARETilePathStartPanel()
{
	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(SceneRoot);
	PanelMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ARETilePathStartPanel::SetTilePathManager(ARETilePathManager* InManager)
{
	SetPuzzleManager(InManager);
}

ARETilePathManager* ARETilePathStartPanel::GetTilePathManager() const
{
	return Cast<ARETilePathManager>(PuzzleManager);
}

void ARETilePathStartPanel::CloseWaitingWidget()
{
	if (IsValid(ActiveWaitingWidget) == true)
	{
		ActiveWaitingWidget->RemoveFromParent();
		ActiveWaitingWidget = nullptr;
	}
}

void ARETilePathStartPanel::ServerCancelWalker_Implementation()
{
	ARETilePathManager* Manager = GetTilePathManager();
	APawn* RequestingPawn = nullptr;
	if (APlayerController* RequestingController = Cast<APlayerController>(GetOwner()))
	{
		RequestingPawn = RequestingController->GetPawn();
	}

	if (HasAuthority() == true && IsValid(Manager) == true && IsValid(RequestingPawn) == true)
	{
		Manager->CancelWalker(RequestingPawn);
	}

	ClientCloseWaitingWidget();
}

bool ARETilePathStartPanel::CanUseElement(AActor* Interactor) const
{
	APawn* Pawn = ResolvePawn(Interactor);
	ARETilePathManager* Manager = GetTilePathManager();
	return IsValid(Manager) == true && IsValid(Pawn) == true && Manager->CanStartPuzzle(Pawn) == true;
}

void ARETilePathStartPanel::HandleInteract(AActor* Interactor)
{
	APawn* Pawn = ResolvePawn(Interactor);
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Pawn) == false || IsValid(PlayerController) == false || IsValid(Manager) == false)
	{
		return;
	}

	SetOwner(PlayerController);
	if (Manager->StartPuzzle(Pawn) == true)
	{
		ClientOpenWaitingWidget(PlayerController);
	}
}

void ARETilePathStartPanel::ClientOpenWaitingWidget_Implementation(APlayerController* OwningPlayerController)
{
	OpenWaitingWidgetLocal(OwningPlayerController);
}

void ARETilePathStartPanel::ClientCloseWaitingWidget_Implementation()
{
	CloseWaitingWidget();
}

APawn* ARETilePathStartPanel::ResolvePawn(AActor* Actor) const
{
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		return PlayerController->GetPawn();
	}

	return nullptr;
}

APlayerController* ARETilePathStartPanel::ResolvePlayerController(AActor* Actor) const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		return PlayerController;
	}

	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Cast<APlayerController>(Pawn->GetController());
	}

	return nullptr;
}

void ARETilePathStartPanel::OpenWaitingWidgetLocal(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(WaitingWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveWaitingWidget) == false)
	{
		ActiveWaitingWidget = CreateWidget<URETilePathWaitingWidget>(PlayerController, WaitingWidgetClass);
	}

	if (IsValid(ActiveWaitingWidget) == false)
	{
		return;
	}

	if (ActiveWaitingWidget->IsInViewport() == false)
	{
		ActiveWaitingWidget->AddToPlayerScreen();
	}

	ActiveWaitingWidget->InitializeWaiting(this, GetTilePathManager(), ERETilePathParticipantRole::Walker);
}
