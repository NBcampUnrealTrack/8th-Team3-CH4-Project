#include "UI/REDialLockWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Puzzles/LockAndGlow/REDialLockDevice.h"
#include "Puzzles/LockAndGlow/RELockAndGlowClueManager.h"
#include "Sound/SoundBase.h"
#include "CommonButtonBase.h"

void UREDialLockWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UREDialLockWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ResolveWidgets();
	BindButtonEvents();
}

void UREDialLockWidget::NativeOnDeactivated()
{
	ClearCloseTimer();
	UnbindDeviceEvents();
	RestoreInput();
	LockDevice = nullptr;

	Super::NativeOnDeactivated();
}

void UREDialLockWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UREDialLockWidget::InitializeDialLock(AREDialLockDevice* InLockDevice, const TArray<int32>& InitialDigits)
{
	ClearCloseTimer();
	UnbindDeviceEvents();

	LockDevice = InLockDevice;
	CachedDigits = InitialDigits;
	bReceivedResultForCurrentOpen = false;
	LastResultMessage = FText::GetEmpty();
	bLastResultCorrect = false;

	BindDeviceEvents();
	CaptureInput();
	ApplyDigitsToText();
	ClearFeedbackText();
	ReceiveDigitsChanged(CachedDigits);
	ReceiveDialLockInitialized();
}

void UREDialLockWidget::RequestIncrementDigit(int32 DigitIndex)
{
	if (IsValid(LockDevice) == true && LockDevice->IsUnlocked() == false)
	{
		PlayDigitStepSound(DigitIndex, true);
		LockDevice->ServerIncrementDigit(DigitIndex);
	}
}

void UREDialLockWidget::RequestDecrementDigit(int32 DigitIndex)
{
	if (IsValid(LockDevice) == true && LockDevice->IsUnlocked() == false)
	{
		PlayDigitStepSound(DigitIndex, false);
		LockDevice->ServerDecrementDigit(DigitIndex);
	}
}

void UREDialLockWidget::RequestSetDigit(int32 DigitIndex, int32 DigitValue)
{
	if (IsValid(LockDevice) == true && LockDevice->IsUnlocked() == false)
	{
		LockDevice->ServerSetDigit(DigitIndex, DigitValue);
	}
}

void UREDialLockWidget::RequestSubmitCode()
{
	if (IsValid(LockDevice) == true && LockDevice->IsUnlocked() == false)
	{
		LockDevice->ServerSubmitCode(GetOwningPlayerPawn());
	}
}

void UREDialLockWidget::CloseDialLock()
{
	ClearCloseTimer();
	RestoreInput();
	DeactivateWidget();
	//if (IsInViewport() == true)
	//{
	//	RemoveFromParent();
	//}
}

AREDialLockDevice* UREDialLockWidget::GetLockDevice() const
{
	return LockDevice;
}

void UREDialLockWidget::HandleDigitsChanged(const TArray<int32>& NewDigits)
{
	CachedDigits = NewDigits;
	ApplyDigitsToText();
	ClearFeedbackText();
	ReceiveDigitsChanged(CachedDigits);
}

void UREDialLockWidget::HandleUnlockedChanged(bool bNewUnlocked)
{
	if (bNewUnlocked == true && bReceivedResultForCurrentOpen == false)
	{
		DisplaySubmitResult(true, FText::FromString(TEXT("자물쇠가 열렸습니다.")));
	}
}

void UREDialLockWidget::HandleInputResult(AActor* SourceActor, bool bCorrect, FText ResultMessage)
{
	if (IsValid(LockDevice) == false || SourceActor != LockDevice)
	{
		return;
	}

	DisplaySubmitResult(bCorrect, ResultMessage);
}

void UREDialLockWidget::HandleDigit0UpClicked()
{
	RequestIncrementDigit(0);
}

void UREDialLockWidget::HandleDigit1UpClicked()
{
	RequestIncrementDigit(1);
}

void UREDialLockWidget::HandleDigit2UpClicked()
{
	RequestIncrementDigit(2);
}

void UREDialLockWidget::HandleDigit3UpClicked()
{
	RequestIncrementDigit(3);
}

void UREDialLockWidget::HandleDigit0DownClicked()
{
	RequestDecrementDigit(0);
}

void UREDialLockWidget::HandleDigit1DownClicked()
{
	RequestDecrementDigit(1);
}

void UREDialLockWidget::HandleDigit2DownClicked()
{
	RequestDecrementDigit(2);
}

void UREDialLockWidget::HandleDigit3DownClicked()
{
	RequestDecrementDigit(3);
}

void UREDialLockWidget::HandleSubmitClicked()
{
	RequestSubmitCode();
}

void UREDialLockWidget::HandleCloseClicked()
{
	CloseDialLock();
}

void UREDialLockWidget::ResolveWidgets()
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (IsValid(TXT_Digit0) == false)
	{
		TXT_Digit0 = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Digit0")));
	}
	if (IsValid(TXT_Digit1) == false)
	{
		TXT_Digit1 = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Digit1")));
	}
	if (IsValid(TXT_Digit2) == false)
	{
		TXT_Digit2 = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Digit2")));
	}
	if (IsValid(TXT_Digit3) == false)
	{
		TXT_Digit3 = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Digit3")));
	}
	if (IsValid(TXT_Feedback) == false)
	{
		TXT_Feedback = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Feedback")));
	}

	if (IsValid(BTN_Digit0Up) == false)
	{
		BTN_Digit0Up = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit0Up")));
	}
	if (IsValid(BTN_Digit1Up) == false)
	{
		BTN_Digit1Up = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit1Up")));
	}
	if (IsValid(BTN_Digit2Up) == false)
	{
		BTN_Digit2Up = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit2Up")));
	}
	if (IsValid(BTN_Digit3Up) == false)
	{
		BTN_Digit3Up = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit3Up")));
	}

	if (IsValid(BTN_Digit0Down) == false)
	{
		BTN_Digit0Down = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit0Down")));
	}
	if (IsValid(BTN_Digit1Down) == false)
	{
		BTN_Digit1Down = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit1Down")));
	}
	if (IsValid(BTN_Digit2Down) == false)
	{
		BTN_Digit2Down = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit2Down")));
	}
	if (IsValid(BTN_Digit3Down) == false)
	{
		BTN_Digit3Down = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Digit3Down")));
	}

	if (IsValid(BTN_Submit) == false)
	{
		BTN_Submit = Cast<UCommonButtonBase>(WidgetTree->FindWidget(TEXT("BTN_Submit")));
	}
	if (IsValid(BTN_Close) == false)
	{
		BTN_Close = Cast<UCommonButtonBase>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	}
}

void UREDialLockWidget::BindButtonEvents()
{
	if (IsValid(BTN_Digit0Up) == true)
	{
		BTN_Digit0Up->OnClicked.RemoveAll(this);
		BTN_Digit0Up->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit0UpClicked);
	}
	if (IsValid(BTN_Digit1Up) == true)
	{
		BTN_Digit1Up->OnClicked.RemoveAll(this);
		BTN_Digit1Up->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit1UpClicked);
	}
	if (IsValid(BTN_Digit2Up) == true)
	{
		BTN_Digit2Up->OnClicked.RemoveAll(this);
		BTN_Digit2Up->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit2UpClicked);
	}
	if (IsValid(BTN_Digit3Up) == true)
	{
		BTN_Digit3Up->OnClicked.RemoveAll(this);
		BTN_Digit3Up->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit3UpClicked);
	}

	if (IsValid(BTN_Digit0Down) == true)
	{
		BTN_Digit0Down->OnClicked.RemoveAll(this);
		BTN_Digit0Down->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit0DownClicked);
	}
	if (IsValid(BTN_Digit1Down) == true)
	{
		BTN_Digit1Down->OnClicked.RemoveAll(this);
		BTN_Digit1Down->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit1DownClicked);
	}
	if (IsValid(BTN_Digit2Down) == true)
	{
		BTN_Digit2Down->OnClicked.RemoveAll(this);
		BTN_Digit2Down->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit2DownClicked);
	}
	if (IsValid(BTN_Digit3Down) == true)
	{
		BTN_Digit3Down->OnClicked.RemoveAll(this);
		BTN_Digit3Down->OnClicked.AddDynamic(this, &UREDialLockWidget::HandleDigit3DownClicked);
	}

	if (IsValid(BTN_Submit) == true)
	{
		BTN_Submit->OnClicked().RemoveAll(this);
		BTN_Submit->OnClicked().AddUObject(this, &UREDialLockWidget::HandleSubmitClicked);
	}
	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked().RemoveAll(this);
		BTN_Close->OnClicked().AddUObject(this, &UREDialLockWidget::HandleCloseClicked);
	}
}

void UREDialLockWidget::BindDeviceEvents()
{
	if (IsValid(LockDevice) == false)
	{
		return;
	}

	LockDevice->OnDialLockDigitsChanged.RemoveDynamic(this, &UREDialLockWidget::HandleDigitsChanged);
	LockDevice->OnDialLockDigitsChanged.AddDynamic(this, &UREDialLockWidget::HandleDigitsChanged);

	LockDevice->OnDialLockUnlockedChanged.RemoveDynamic(this, &UREDialLockWidget::HandleUnlockedChanged);
	LockDevice->OnDialLockUnlockedChanged.AddDynamic(this, &UREDialLockWidget::HandleUnlockedChanged);

	if (ARELockAndGlowClueManager* LockManager = LockDevice->GetLockAndGlowManager())
	{
		LockManager->OnDialLockInputResult.RemoveDynamic(this, &UREDialLockWidget::HandleInputResult);
		LockManager->OnDialLockInputResult.AddDynamic(this, &UREDialLockWidget::HandleInputResult);
		BoundLockManager = LockManager;
	}
}

void UREDialLockWidget::UnbindDeviceEvents()
{
	if (IsValid(LockDevice) == true)
	{
		LockDevice->OnDialLockDigitsChanged.RemoveDynamic(this, &UREDialLockWidget::HandleDigitsChanged);
		LockDevice->OnDialLockUnlockedChanged.RemoveDynamic(this, &UREDialLockWidget::HandleUnlockedChanged);
	}

	if (ARELockAndGlowClueManager* LockManager = BoundLockManager.Get())
	{
		LockManager->OnDialLockInputResult.RemoveDynamic(this, &UREDialLockWidget::HandleInputResult);
	}
	BoundLockManager.Reset();
}

void UREDialLockWidget::ApplyDigitsToText()
{
	ResolveWidgets();

	UTextBlock* DigitTextBlocks[] = { TXT_Digit0.Get(), TXT_Digit1.Get(), TXT_Digit2.Get(), TXT_Digit3.Get() };
	for (int32 DigitIndex = 0; DigitIndex < UE_ARRAY_COUNT(DigitTextBlocks); ++DigitIndex)
	{
		if (IsValid(DigitTextBlocks[DigitIndex]) == false)
		{
			continue;
		}

		const int32 DigitValue = CachedDigits.IsValidIndex(DigitIndex) == true ? FMath::Clamp(CachedDigits[DigitIndex], 0, 9) : 0;
		DigitTextBlocks[DigitIndex]->SetText(FText::AsNumber(DigitValue));
	}
}

void UREDialLockWidget::ClearFeedbackText()
{
	ResolveWidgets();
	if (IsValid(TXT_Feedback) == true)
	{
		TXT_Feedback->SetText(FText::GetEmpty());
		TXT_Feedback->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UREDialLockWidget::DisplaySubmitResult(bool bCorrect, const FText& ResultMessage)
{
	bReceivedResultForCurrentOpen = true;
	bLastResultCorrect = bCorrect;
	LastResultMessage = ResultMessage;

	ResolveWidgets();
	if (IsValid(TXT_Feedback) == true)
	{
		TXT_Feedback->SetText(ResultMessage);
		TXT_Feedback->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	ReceiveSubmitResult(bCorrect, ResultMessage);

	if (bCorrect == true)
	{
		ScheduleCloseAfterCorrectResult();
	}
}

void UREDialLockWidget::ScheduleCloseAfterCorrectResult()
{
	if (bCloseAfterCorrectResult == false)
	{
		return;
	}

	if (CorrectResultCloseDelay <= 0.0f)
	{
		CloseDialLock();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CorrectResultCloseTimerHandle, this, &UREDialLockWidget::CloseDialLock, CorrectResultCloseDelay, false);
	}
}

void UREDialLockWidget::ClearCloseTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CorrectResultCloseTimerHandle);
	}
}

void UREDialLockWidget::PlayDigitStepSound(int32 DigitIndex, bool bIncrementing)
{
	ReceiveDigitStepRequested(DigitIndex, bIncrementing);
	PlayUISound(bIncrementing == true ? DigitUpSound : DigitDownSound);
}

void UREDialLockWidget::PlayUISound(USoundBase* Sound) const
{
	if (IsValid(Sound) == true)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void UREDialLockWidget::CaptureInput()
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

void UREDialLockWidget::RestoreInput()
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
