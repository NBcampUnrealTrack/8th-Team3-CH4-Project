#include "Puzzles/TilePath/RETilePathMonitor.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/TilePath/RETilePathManager.h"
#include "UI/RETilePathMonitorWidget.h"
#include "UI/RETilePathWaitingWidget.h"
#include "UI/LocalWidgetManager.h"
#include "UI/RERootCanvasWidget.h"

ARETilePathMonitor::ARETilePathMonitor()
{
	MonitorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorMesh"));
	MonitorMesh->SetupAttachment(SceneRoot);
	MonitorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(100.0, 100.0, 120.0));
}

void ARETilePathMonitor::SetTilePathManager(ARETilePathManager* InManager)
{
	SetPuzzleManager(InManager);
}

ARETilePathManager* ARETilePathMonitor::GetTilePathManager() const
{
	return Cast<ARETilePathManager>(PuzzleManager);
}

bool ARETilePathMonitor::GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const
{
	OutQuestionId = NAME_None;
	ARETilePathManager* Manager = GetTilePathManager();
	return IsValid(Manager) == true && Manager->GetCurrentQuestionData(OutQuestionId, OutQuestionRow) == true;
}

bool ARETilePathMonitor::GetQuestionData(FName QuestionId, FRETilePathQuestionRow& OutQuestionRow) const
{
	ARETilePathManager* Manager = GetTilePathManager();
	return IsValid(Manager) == true && Manager->GetQuestionData(QuestionId, OutQuestionRow) == true;
}

void ARETilePathMonitor::GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const
{
	OutRevealedMoves.Reset();

	if (ARETilePathManager* Manager = GetTilePathManager())
	{
		Manager->GetRevealedMoves(OutRevealedMoves);
	}
}

FName ARETilePathMonitor::GetCurrentQuestionId() const
{
	ARETilePathManager* Manager = GetTilePathManager();
	return IsValid(Manager) == true ? Manager->GetCurrentQuestionId() : NAME_None;
}

void ARETilePathMonitor::CloseMonitorWidget()
{
	if (IsValid(ActiveMonitorWidget) == true)
	{
		//ActiveMonitorWidget->RemoveFromParent();
		//ActiveMonitorWidget = nullptr;
		ActiveMonitorWidget->DeactivateWidget();
	}
}

void ARETilePathMonitor::CloseWaitingWidget()
{
	if (IsValid(ActiveWaitingWidget) == true)
	{
		ActiveWaitingWidget->RemoveFromParent();
		ActiveWaitingWidget = nullptr;
	}
}

void ARETilePathMonitor::OpenMonitorWidgetLocal(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(MonitorWidgetClass) == false)
	{
		return;
	}

	CloseWaitingWidget();

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	URERootCanvasWidget* RootCanvas = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvas) == false)
	{
		return;
	}

	ActiveMonitorWidget = Cast<URETilePathMonitorWidget>(RootCanvas->ShowGameplayWidget(MonitorWidgetClass));
	if (IsValid(ActiveMonitorWidget) == false)
	{
		//ActiveMonitorWidget = CreateWidget<URETilePathMonitorWidget>(PlayerController, MonitorWidgetClass);
	}

	if (IsValid(ActiveMonitorWidget) == false)
	{
		return;
	}

	//if (ActiveMonitorWidget->IsInViewport() == false)
	//{
	//	ActiveMonitorWidget->AddToPlayerScreen();
	//}

	ActiveMonitorWidget->InitializeMonitor(this);
}

void ARETilePathMonitor::ServerSubmitAnswer_Implementation(FName QuestionId, ERETilePathAnswerChoice SubmittedAnswer)
{
	ARETilePathManager* Manager = GetTilePathManager();
	if (HasAuthority() == false || IsValid(Manager) == false)
	{
		return;
	}

	APlayerController* RequestingController = Cast<APlayerController>(GetOwner());
	FRETilePathRevealedMove RevealedMove;
	const bool bCorrect = Manager->SubmitAnswer(RequestingController, QuestionId, SubmittedAnswer, RevealedMove);
	ClientReceiveAnswerResult(bCorrect, RevealedMove, Manager->GetCurrentQuestionId());
}

void ARETilePathMonitor::ServerCancelGuide_Implementation()
{
	ARETilePathManager* Manager = GetTilePathManager();
	APlayerController* RequestingController = Cast<APlayerController>(GetOwner());
	if (HasAuthority() == true && IsValid(Manager) == true && IsValid(RequestingController) == true)
	{
		Manager->CancelGuide(RequestingController);
	}
	ClientCloseWaitingWidget();
	ClientCloseMonitorWidget();
}

bool ARETilePathMonitor::CanUseElement(AActor* Interactor) const
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(PlayerController) == false || IsValid(Manager) == false || Manager->IsActive() == false)
	{
		return false;
	}

	if (Manager->IsSessionStarted() == false)
	{
		return true;
	}

	return Manager->GetGuidePlayerState() == PlayerController->PlayerState;
}

void ARETilePathMonitor::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(PlayerController) == false || IsValid(Manager) == false)
	{
		return;
	}

	SetOwner(PlayerController);
	if (Manager->IsSessionStarted() == true)
	{
		ClientOpenMonitorWidget(PlayerController);
		return;
	}

	if (Manager->RegisterGuide(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TilePath] Monitor interact failed. Monitor=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(PlayerController));
		return;
	}

	ClientOpenWaitingWidget(PlayerController);
}

void ARETilePathMonitor::ClientOpenMonitorWidget_Implementation(APlayerController* OwningPlayerController)
{
	APlayerController* PlayerController = OwningPlayerController;
	if (IsValid(PlayerController) == true)
	{
		SetOwner(PlayerController);
	}

	OpenMonitorWidgetLocal(PlayerController);
}

void ARETilePathMonitor::ClientOpenWaitingWidget_Implementation(APlayerController* OwningPlayerController)
{
	APlayerController* PlayerController = OwningPlayerController;
	if (IsValid(PlayerController) == true)
	{
		SetOwner(PlayerController);
	}

	OpenWaitingWidgetLocal(PlayerController);
}

void ARETilePathMonitor::ClientReceiveAnswerResult_Implementation(bool bCorrect, const FRETilePathRevealedMove& RevealedMove, FName NextQuestionId)
{
	if (IsValid(ActiveMonitorWidget) == true)
	{
		ActiveMonitorWidget->HandleAnswerResult(bCorrect, RevealedMove, NextQuestionId);
	}
}

void ARETilePathMonitor::ClientCloseMonitorWidget_Implementation()
{
	CloseMonitorWidget();
}

void ARETilePathMonitor::ClientCloseWaitingWidget_Implementation()
{
	CloseWaitingWidget();
}

APlayerController* ARETilePathMonitor::ResolvePlayerController(AActor* Actor) const
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

void ARETilePathMonitor::OpenWaitingWidgetLocal(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
	{
		return;
	}

	if (IsValid(WaitingWidgetClass) == false)
	{
		OpenMonitorWidgetLocal(PlayerController);
		return;
	}

	CloseMonitorWidget();

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	URERootCanvasWidget* RootCanvas = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvas) == false)
	{
		return;
	}

	ActiveWaitingWidget = Cast<URETilePathWaitingWidget>(RootCanvas->ShowGameplayWidget(WaitingWidgetClass));

	if (IsValid(ActiveWaitingWidget) == false)
	{
		//ActiveWaitingWidget = CreateWidget<URETilePathWaitingWidget>(PlayerController, WaitingWidgetClass);
	}

	if (IsValid(ActiveWaitingWidget) == false)
	{
		return;
	}

	//if (ActiveWaitingWidget->IsInViewport() == false)
	//{
	//	ActiveWaitingWidget->AddToPlayerScreen();
	//}

	ActiveWaitingWidget->InitializeWaiting(this, GetTilePathManager(), ERETilePathParticipantRole::Guide);
}
