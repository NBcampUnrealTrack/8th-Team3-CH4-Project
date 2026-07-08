#include "Puzzles/MirrorRoom/REMirrorInputPanel.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/MirrorRoom/REMirrorRoomManager.h"
#include "Puzzles/MirrorRoom/REMirrorPuzzleData.h"
#include "UI/REMirrorInputWidget.h"

AREMirrorInputPanel::AREMirrorInputPanel()
{
	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(SceneRoot);
	PanelMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(120.0, 80.0, 100.0));
	InteractionPromptText = FText::FromString(TEXT("입력 패널"));
	InteractionPromptRelativeLocation = FVector(0.0, 0.0, 120.0);
}

void AREMirrorInputPanel::SetMirrorManager(AREMirrorRoomManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREMirrorRoomManager* AREMirrorInputPanel::GetMirrorManager() const
{
	return Cast<AREMirrorRoomManager>(PuzzleManager);
}

void AREMirrorInputPanel::CloseInputWidget()
{
	if (IsValid(ActiveInputWidget) == true)
	{
		ActiveInputWidget->RemoveFromParent();
		ActiveInputWidget = nullptr;
	}
}

void AREMirrorInputPanel::SubmitInputId(const FName& InputId, AActor* Interactor)
{
	if (HasAuthority() == false)
	{
		ServerSubmitInputId(InputId, Interactor);
		return;
	}

	FREMirrorInputResult Result;
	if (AREMirrorRoomManager* MirrorManager = GetMirrorManager())
	{
		MirrorManager->SubmitInputId(this, Interactor, InputId, Result);
	}
	DeliverInputResultToOwner(Interactor, Result);
}

void AREMirrorInputPanel::SubmitClockHour(int32 ClockHour, AActor* Interactor)
{
	if (HasAuthority() == false)
	{
		ServerSubmitClockHour(ClockHour, Interactor);
		return;
	}

	FREMirrorInputResult Result;
	if (AREMirrorRoomManager* MirrorManager = GetMirrorManager())
	{
		MirrorManager->SubmitClockHour(this, Interactor, ClockHour, Result);
	}
	DeliverInputResultToOwner(Interactor, Result);
}

void AREMirrorInputPanel::RequestInputReset(AActor* Interactor)
{
	if (HasAuthority() == false)
	{
		ServerRequestInputReset(Interactor);
		return;
	}

	FREMirrorInputResult Result;
	if (AREMirrorRoomManager* MirrorManager = GetMirrorManager())
	{
		MirrorManager->ResetInput(this, Interactor, Result);
	}
	DeliverInputResultToOwner(Interactor, Result);
}

void AREMirrorInputPanel::ServerSubmitInputId_Implementation(const FName& InputId, AActor* Interactor)
{
	SubmitInputId(InputId, Interactor);
}

void AREMirrorInputPanel::ServerSubmitClockHour_Implementation(int32 ClockHour, AActor* Interactor)
{
	SubmitClockHour(ClockHour, Interactor);
}

void AREMirrorInputPanel::ServerRequestInputReset_Implementation(AActor* Interactor)
{
	RequestInputReset(Interactor);
}

bool AREMirrorInputPanel::CanUseElement(AActor* Interactor) const
{
	return Super::CanUseElement(Interactor) == true && IsValid(InputWidgetClass) == true && IsValid(GetMirrorManager()) == true && IsValid(GetMirrorManager()->GetMirrorPuzzleData()) == true;
}

void AREMirrorInputPanel::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	if (IsValid(PlayerController) == false || IsValid(InputWidgetClass) == false)
	{
		return;
	}

	SetOwner(PlayerController);
	if (PlayerController->IsLocalController() == true)
	{
		OpenInputWidgetLocal(PlayerController, InputWidgetClass, InputPanelTitle);
		return;
	}

	ClientOpenInputWidget(PlayerController, InputWidgetClass, InputPanelTitle);
}

void AREMirrorInputPanel::ClientOpenInputWidget_Implementation(APlayerController* OwningPlayerController, TSubclassOf<UREMirrorInputWidget> InWidgetClass, const FText& InPanelTitle)
{
	OpenInputWidgetLocal(OwningPlayerController, InWidgetClass, InPanelTitle);
}

void AREMirrorInputPanel::ClientReceiveInputResult_Implementation(const FREMirrorInputResult& Result)
{
	if (IsValid(ActiveInputWidget) == true)
	{
		ActiveInputWidget->ApplyInputResult(Result);
	}
}

APlayerController* AREMirrorInputPanel::ResolvePlayerController(AActor* Actor) const
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

void AREMirrorInputPanel::OpenInputWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREMirrorInputWidget> InWidgetClass, const FText& InPanelTitle)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(InWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveInputWidget) == false || ActiveInputWidget->GetClass() != InWidgetClass)
	{
		ActiveInputWidget = CreateWidget<UREMirrorInputWidget>(PlayerController, InWidgetClass);
	}

	if (IsValid(ActiveInputWidget) == false)
	{
		return;
	}

	if (ActiveInputWidget->IsInViewport() == false)
	{
		ActiveInputWidget->AddToViewport(100);
	}

	ActiveInputWidget->InitializeInputPanel(this, InPanelTitle);
}

void AREMirrorInputPanel::DeliverInputResultToOwner(AActor* Interactor, const FREMirrorInputResult& Result)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	SetOwner(PlayerController);
	if (PlayerController->IsLocalController() == true)
	{
		if (IsValid(ActiveInputWidget) == true)
		{
			ActiveInputWidget->ApplyInputResult(Result);
		}
		return;
	}

	ClientReceiveInputResult(Result);
}
