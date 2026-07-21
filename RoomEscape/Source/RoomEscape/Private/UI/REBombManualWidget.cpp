#include "UI/REBombManualWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/BombDefusal/REDefusalManual.h"

void UREBombManualWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UREBombManualWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (IsValid(WidgetTree) == true)
	{
		if (IsValid(TXT_ManualTitle) == false)
		{
			TXT_ManualTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ManualTitle")));
		}
		if (IsValid(TXT_ManualRules) == false)
		{
			TXT_ManualRules = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ManualRules")));
		}
		if (IsValid(BTN_Close) == false)
		{
			BTN_Close = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Close")));
		}
	}

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked.RemoveAll(this);
		BTN_Close->OnClicked.AddDynamic(this, &UREBombManualWidget::HandleCloseClicked);
	}
}

void UREBombManualWidget::NativeOnDeactivated()
{
	RestoreInput();
	ManualActor = nullptr;

	Super::NativeOnDeactivated();
}

void UREBombManualWidget::NativeDestruct()
{

	Super::NativeDestruct();
}

void UREBombManualWidget::InitializeManual(AREDefusalManual* InManualActor, const FText& InManualTitle, const FText& InManualText)
{
	ManualActor = InManualActor;
	ManualTitle = InManualTitle;
	ManualText = InManualText;
	CaptureInput();
	ApplyManualText();
	ReceiveManualInitialized();
}

void UREBombManualWidget::CloseManual()
{
	RestoreInput();
	DeactivateWidget();
	//if (IsInViewport() == true)
	//{
	//	RemoveFromParent();
	//}
}

AREDefusalManual* UREBombManualWidget::GetManualActor() const
{
	return ManualActor;
}

void UREBombManualWidget::HandleCloseClicked()
{
	CloseManual();
}

void UREBombManualWidget::ApplyManualText()
{
	if (IsValid(WidgetTree) == true)
	{
		if (IsValid(TXT_ManualTitle) == false)
		{
			TXT_ManualTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ManualTitle")));
		}
		if (IsValid(TXT_ManualRules) == false)
		{
			TXT_ManualRules = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ManualRules")));
		}
	}

	if (IsValid(TXT_ManualTitle) == true)
	{
		TXT_ManualTitle->SetText(ManualTitle);
	}
	if (IsValid(TXT_ManualRules) == true)
	{
		TXT_ManualRules->SetText(ManualText);
	}
}

void UREBombManualWidget::CaptureInput()
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

void UREBombManualWidget::RestoreInput()
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
