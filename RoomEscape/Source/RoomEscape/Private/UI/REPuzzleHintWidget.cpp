#include "UI/REPuzzleHintWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/Framework/REPuzzleHintActor.h"

void UREPuzzleHintWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//if (IsValid(BTN_Close) == false && IsValid(WidgetTree) == true)
	//{
	//	BTN_Close = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	//}

	//if (IsValid(BTN_Close) == true)
	//{
	//	BTN_Close->OnClicked.RemoveAll(this);
	//	BTN_Close->OnClicked.AddDynamic(this, &UREPuzzleHintWidget::HandleCloseClicked);
	//}
}

void UREPuzzleHintWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (IsValid(BTN_Close) == false && IsValid(WidgetTree) == true)
	{
		BTN_Close = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	}

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked.RemoveAll(this);
		BTN_Close->OnClicked.AddDynamic(this, &UREPuzzleHintWidget::HandleCloseClicked);
	}
}

void UREPuzzleHintWidget::NativeOnDeactivated()
{
	RestoreInput();
	HintActor = nullptr;

	Super::NativeOnDeactivated();
}

void UREPuzzleHintWidget::NativeDestruct()
{
	//RestoreInput();
	//HintActor = nullptr;
	Super::NativeDestruct();
}

void UREPuzzleHintWidget::InitializeHint(AREPuzzleHintActor* InHintActor, const FText& InHintTitle, const FText& InHintText)
{
	HintActor = InHintActor;
	HintTitle = InHintTitle;
	HintText = InHintText;
	CaptureInput();
	ApplyHintText();
	ReceiveHintInitialized();
}

void UREPuzzleHintWidget::CloseHint()
{
	RestoreInput();
	DeactivateWidget();
	//if (IsInViewport() == true)
	//{
	//	RemoveFromParent();
	//}
}

AREPuzzleHintActor* UREPuzzleHintWidget::GetHintActor() const
{
	return HintActor;
}

FText UREPuzzleHintWidget::GetHintTitle() const
{
	return HintTitle;
}

FText UREPuzzleHintWidget::GetHintText() const
{
	return HintText;
}

void UREPuzzleHintWidget::HandleCloseClicked()
{
	CloseHint();
}

void UREPuzzleHintWidget::ApplyHintText()
{
	if (IsValid(WidgetTree) == true)
	{
		if (IsValid(TXT_HintTitle) == false)
		{
			TXT_HintTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_HintTitle")));
		}

		if (IsValid(TXT_HintText) == false)
		{
			TXT_HintText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_HintText")));
		}
	}

	if (IsValid(TXT_HintTitle) == true)
	{
		TXT_HintTitle->SetText(HintTitle);
	}

	if (IsValid(TXT_HintText) == true)
	{
		TXT_HintText->SetText(HintText);
	}
}

void UREPuzzleHintWidget::CaptureInput()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	CapturedPlayerController = PlayerController;
	if (bCapturedInput == false)
	{
		bPreviousMouseCursor = PlayerController->bShowMouseCursor;
	}

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	bCapturedInput = true;
}

void UREPuzzleHintWidget::RestoreInput()
{
	if (bCapturedInput == false)
	{
		return;
	}

	APlayerController* PlayerController = CapturedPlayerController.Get();
	if (IsValid(PlayerController) == false)
	{
		PlayerController = GetOwningPlayer();
	}

	if (IsValid(PlayerController) == false)
	{
		bCapturedInput = false;
		return;
	}

	PlayerController->ResetIgnoreMoveInput();
	PlayerController->ResetIgnoreLookInput();
	PlayerController->bShowMouseCursor = bPreviousMouseCursor;
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->FlushPressedKeys();
	CapturedPlayerController.Reset();
	bCapturedInput = false;
}
