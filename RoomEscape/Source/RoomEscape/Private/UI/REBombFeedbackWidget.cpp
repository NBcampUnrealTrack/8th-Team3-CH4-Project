#include "UI/REBombFeedbackWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UREBombFeedbackWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveWidgets();

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked.RemoveAll(this);
		BTN_Close->OnClicked.AddDynamic(this, &UREBombFeedbackWidget::HandleCloseClicked);
	}
}

void UREBombFeedbackWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
	}

	RestoreInput();
	Super::NativeDestruct();
}

void UREBombFeedbackWidget::InitializeFeedback(AActor* SourceActor, bool bCorrect, const FText& FeedbackMessage, float DisplaySeconds)
{
	ResolveWidgets();

	if (IsValid(TXT_FeedbackTitle) == true)
	{
		TXT_FeedbackTitle->SetText(bCorrect == true ? FText::FromString(TEXT("처리 완료")) : FText::FromString(TEXT("폭발")));
	}

	if (IsValid(TXT_FeedbackMessage) == true)
	{
		TXT_FeedbackMessage->SetText(FeedbackMessage);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
		if (DisplaySeconds > 0.0f)
		{
			World->GetTimerManager().SetTimer(AutoCloseTimerHandle, this, &UREBombFeedbackWidget::CloseFeedback, DisplaySeconds, false);
		}
	}

	CaptureInput();
	ReceiveFeedbackInitialized(SourceActor, bCorrect, FeedbackMessage);
}

void UREBombFeedbackWidget::CloseFeedback()
{
	RestoreInput();
	if (IsInViewport() == true)
	{
		RemoveFromParent();
	}
}

void UREBombFeedbackWidget::HandleCloseClicked()
{
	CloseFeedback();
}

void UREBombFeedbackWidget::ResolveWidgets()
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (IsValid(TXT_FeedbackTitle) == false)
	{
		TXT_FeedbackTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_FeedbackTitle")));
	}

	if (IsValid(TXT_FeedbackMessage) == false)
	{
		TXT_FeedbackMessage = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_FeedbackMessage")));
	}

	if (IsValid(BTN_Close) == false)
	{
		BTN_Close = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	}
}

void UREBombFeedbackWidget::CaptureInput()
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

void UREBombFeedbackWidget::RestoreInput()
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
		CapturedPlayerController.Reset();
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
