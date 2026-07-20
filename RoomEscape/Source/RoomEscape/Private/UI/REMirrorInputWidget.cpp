#include "UI/REMirrorInputWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/MirrorRoom/REMirrorInputPanel.h"
#include "Puzzles/MirrorRoom/REMirrorPuzzleData.h"
#include "Puzzles/MirrorRoom/REMirrorRoomManager.h"
#include "CommonButtonBase.h"

void UREMirrorInputWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UREMirrorInputWidget::NativeOnActivated()
{

	Super::NativeOnActivated();
	FindOptionalWidgets();

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked().RemoveAll(this);
		BTN_Close->OnClicked().AddUObject(this, &UREMirrorInputWidget::HandleCloseClicked);
	}

	if (IsValid(BTN_Reset) == true)
	{
		BTN_Reset->OnClicked().RemoveAll(this);
		BTN_Reset->OnClicked().AddUObject(this, &UREMirrorInputWidget::HandleResetClicked);
	}

	BindOptionButtons();
}

void UREMirrorInputWidget::NativeOnDeactivated()
{
	RestoreInput();
	InputPanel = nullptr;
	Super::NativeOnDeactivated();
}

void UREMirrorInputWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UREMirrorInputWidget::InitializeInputPanel(AREMirrorInputPanel* InInputPanel, const FText& InInputTitle)
{
	InputPanel = InInputPanel;
	InputTitle = InInputTitle;
	bLocalInputCompleted = false;
	LastSubmittedInputText = FText::GetEmpty();
	LastCompletionMessage = FText::GetEmpty();
	CaptureInput();
	RefreshInputView();
	ReceiveInputPanelInitialized();
}

void UREMirrorInputWidget::SubmitInputId(FName InputId)
{
	if (IsValid(InputPanel) == false)
	{
		return;
	}

	InputPanel->SubmitInputId(InputId, GetOwningInteractor());
}

void UREMirrorInputWidget::SubmitClockHour(int32 ClockHour)
{
	if (IsValid(InputPanel) == false)
	{
		return;
	}

	InputPanel->SubmitClockHour(ClockHour, GetOwningInteractor());
}

void UREMirrorInputWidget::SubmitInputOptionByIndex(int32 OptionIndex)
{
	if (CachedInputOptions.IsValidIndex(OptionIndex) == false)
	{
		return;
	}

	const FREMirrorInputOption& Option = CachedInputOptions[OptionIndex];
	if (AREMirrorRoomManager* MirrorManager = GetMirrorManager())
	{
		if (UREMirrorPuzzleData* Data = MirrorManager->GetMirrorPuzzleData())
		{
			if (Data->InputType == EREMirrorInputType::ClockHourSequence)
			{
				SubmitClockHour(Option.ClockHour);
				return;
			}
		}
	}

	SubmitInputId(Option.InputId);
}

void UREMirrorInputWidget::RequestInputReset()
{
	if (IsValid(InputPanel) == true)
	{
		InputPanel->RequestInputReset(GetOwningInteractor());
	}
}

void UREMirrorInputWidget::CloseInputPanel()
{
	RestoreInput();
	DeactivateWidget();
	//if (IsInViewport() == true)
	//{
	//	RemoveFromParent();
	//}
}

void UREMirrorInputWidget::ApplyInputResult(const FREMirrorInputResult& Result)
{
	if (Result.bReset == true)
	{
		bLocalInputCompleted = false;
		LastCompletionMessage = FText::GetEmpty();
	}

	if (Result.bSolved == true)
	{
		bLocalInputCompleted = true;
		LastCompletionMessage = Result.ResultMessage;
	}

	LastSubmittedInputText = Result.SubmittedInputText;

	RefreshInputView();

	if (IsValid(TXT_CurrentInput) == true)
	{
		TXT_CurrentInput->SetText(LastSubmittedInputText);
	}

	if (IsValid(TXT_Result) == true)
	{
		TXT_Result->SetText(Result.ResultMessage);
	}

	RefreshCompletionView();
	ReceiveInputResult(Result);
}

void UREMirrorInputWidget::RefreshInputView()
{
	FindOptionalWidgets();

	if (IsValid(TXT_InputTitle) == true)
	{
		TXT_InputTitle->SetText(InputTitle);
	}

	AREMirrorRoomManager* MirrorManager = GetMirrorManager();
	if (IsValid(MirrorManager) == false)
	{
		if (IsValid(TXT_CurrentInput) == true)
		{
			TXT_CurrentInput->SetText(FText::GetEmpty());
		}
		return;
	}

	if (IsValid(TXT_CurrentInput) == true)
	{
		const FText ManagerInputText = MirrorManager->GetSubmittedInputText();
		TXT_CurrentInput->SetText(LastSubmittedInputText.IsEmpty() == false ? LastSubmittedInputText : ManagerInputText);
	}

	MirrorManager->GetInputOptions(CachedInputOptions);
	RefreshOptionButtons();

	if (IsValid(TXT_InputOptions) == true)
	{
		TArray<FString> OptionLines;
		for (const FREMirrorInputOption& Option : CachedInputOptions)
		{
			OptionLines.Add(Option.DisplayText.IsEmpty() == false ? Option.DisplayText.ToString() : Option.InputId.ToString());
		}
		TXT_InputOptions->SetText(FText::FromString(FString::Join(OptionLines, TEXT("  "))));
	}

	RefreshCompletionView();
}

AREMirrorInputPanel* UREMirrorInputWidget::GetInputPanel() const
{
	return InputPanel;
}

AREMirrorRoomManager* UREMirrorInputWidget::GetMirrorManager() const
{
	return IsValid(InputPanel) == true ? InputPanel->GetMirrorManager() : nullptr;
}

void UREMirrorInputWidget::HandleCloseClicked()
{
	CloseInputPanel();
}

void UREMirrorInputWidget::HandleResetClicked()
{
	RequestInputReset();
}

void UREMirrorInputWidget::HandleOption01Clicked(){ SubmitInputOptionByIndex(0); }
void UREMirrorInputWidget::HandleOption02Clicked(){ SubmitInputOptionByIndex(1); }
void UREMirrorInputWidget::HandleOption03Clicked(){ SubmitInputOptionByIndex(2); }
void UREMirrorInputWidget::HandleOption04Clicked(){ SubmitInputOptionByIndex(3); }
void UREMirrorInputWidget::HandleOption05Clicked(){ SubmitInputOptionByIndex(4); }
void UREMirrorInputWidget::HandleOption06Clicked(){ SubmitInputOptionByIndex(5); }
void UREMirrorInputWidget::HandleOption07Clicked(){ SubmitInputOptionByIndex(6); }
void UREMirrorInputWidget::HandleOption08Clicked(){ SubmitInputOptionByIndex(7); }
void UREMirrorInputWidget::HandleOption09Clicked(){ SubmitInputOptionByIndex(8); }
void UREMirrorInputWidget::HandleOption10Clicked(){ SubmitInputOptionByIndex(9); }
void UREMirrorInputWidget::HandleOption11Clicked(){ SubmitInputOptionByIndex(10); }
void UREMirrorInputWidget::HandleOption12Clicked(){ SubmitInputOptionByIndex(11); }

void UREMirrorInputWidget::FindOptionalWidgets()
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (IsValid(TXT_InputTitle) == false)
	{
		TXT_InputTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_InputTitle")));
	}
	if (IsValid(TXT_CurrentInput) == false)
	{
		TXT_CurrentInput = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_CurrentInput")));
	}
	if (IsValid(TXT_Result) == false)
	{
		TXT_Result = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Result")));
	}
	if (IsValid(TXT_ClearMessage) == false)
	{
		TXT_ClearMessage = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ClearMessage")));
	}
	if (IsValid(TXT_InputOptions) == false)
	{
		TXT_InputOptions = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_InputOptions")));
	}
	if (IsValid(BTN_Close) == false)
	{
		BTN_Close = Cast<UCommonButtonBase>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	}
	if (IsValid(BTN_Reset) == false)
	{
		BTN_Reset = Cast<UCommonButtonBase>(WidgetTree->FindWidget(TEXT("BTN_Reset")));
	}
}

void UREMirrorInputWidget::BindOptionButtons()
{
	for (int32 Index = 0; Index < 12; ++Index)
	{
		BindOptionButton(Index, FindButtonByOptionIndex(Index));
	}
}

void UREMirrorInputWidget::BindOptionButton(int32 Index, UButton* Button)
{
	if (IsValid(Button) == false)
	{
		return;
	}

	Button->OnClicked.RemoveAll(this);
	switch (Index)
	{
	case 0: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption01Clicked); break;
	case 1: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption02Clicked); break;
	case 2: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption03Clicked); break;
	case 3: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption04Clicked); break;
	case 4: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption05Clicked); break;
	case 5: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption06Clicked); break;
	case 6: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption07Clicked); break;
	case 7: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption08Clicked); break;
	case 8: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption09Clicked); break;
	case 9: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption10Clicked); break;
	case 10: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption11Clicked); break;
	case 11: Button->OnClicked.AddDynamic(this, &UREMirrorInputWidget::HandleOption12Clicked); break;
	default: break;
	}
}

void UREMirrorInputWidget::RefreshOptionButtons()
{
	for (int32 Index = 0; Index < 12; ++Index)
	{
		const bool bHasOption = CachedInputOptions.IsValidIndex(Index);
		const bool bCompleted = bLocalInputCompleted == true || (IsValid(GetMirrorManager()) == true && GetMirrorManager()->IsMirrorInputComplete() == true);
		const FText OptionText = bHasOption == true ? CachedInputOptions[Index].DisplayText : FText::GetEmpty();
		SetOptionButton(Index, bHasOption == true && bCompleted == false, OptionText);
	}
}

void UREMirrorInputWidget::RefreshCompletionView()
{
	const AREMirrorRoomManager* MirrorManager = GetMirrorManager();
	const bool bCompleted = bLocalInputCompleted == true || (IsValid(MirrorManager) == true && MirrorManager->IsMirrorInputComplete() == true);
	const ESlateVisibility InputVisibility = bCompleted == true ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	const ESlateVisibility ClearVisibility = bCompleted == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (IsValid(TXT_ClearMessage) == true)
	{
		if (bCompleted == true)
		{
			TXT_ClearMessage->SetText(LastCompletionMessage.IsEmpty() == false ? LastCompletionMessage : FText::FromString(TEXT("CLEAR\n입력이 완료되었습니다.")));
		}
		TXT_ClearMessage->SetVisibility(ClearVisibility);
	}

	if (IsValid(TXT_InputOptions) == true)
	{
		TXT_InputOptions->SetVisibility(InputVisibility);
	}

	if (bCompleted == false)
	{
		return;
	}

	for (int32 Index = 0; Index < 12; ++Index)
	{
		SetWidgetVisibilityByName(*FString::Printf(TEXT("BTN_Input%02d"), Index + 1), ESlateVisibility::Collapsed);
	}
}


void UREMirrorInputWidget::SetOptionButton(int32 Index, bool bVisible, const FText& OptionText)
{
	UButton* Button = FindButtonByOptionIndex(Index);
	if (IsValid(Button) == true)
	{
		Button->SetVisibility(bVisible == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	SetTextByName(*FString::Printf(TEXT("TXT_Input%02d"), Index + 1), OptionText);
}

void UREMirrorInputWidget::SetTextByName(const FName& WidgetName, const FText& TextValue)
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (UTextBlock* TextBlock = Cast<UTextBlock>(WidgetTree->FindWidget(WidgetName)))
	{
		TextBlock->SetText(TextValue);
	}
}

void UREMirrorInputWidget::SetWidgetVisibilityByName(const FName& WidgetName, ESlateVisibility NewVisibility)
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (UWidget* Widget = WidgetTree->FindWidget(WidgetName))
	{
		Widget->SetVisibility(NewVisibility);
	}
}

UButton* UREMirrorInputWidget::FindButtonByOptionIndex(int32 Index) const
{
	if (IsValid(WidgetTree) == false)
	{
		return nullptr;
	}

	return Cast<UButton>(WidgetTree->FindWidget(*FString::Printf(TEXT("BTN_Input%02d"), Index + 1)));
}

AActor* UREMirrorInputWidget::GetOwningInteractor() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController) == false)
	{
		return nullptr;
	}

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		return Pawn;
	}

	return PlayerController;
}

void UREMirrorInputWidget::CaptureInput()
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

void UREMirrorInputWidget::RestoreInput()
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
