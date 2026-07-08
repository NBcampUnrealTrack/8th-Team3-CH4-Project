#include "UI/RETilePathMonitorWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/TilePath/RETilePathFunctionLibrary.h"
#include "Puzzles/TilePath/RETilePathManager.h"
#include "Puzzles/TilePath/RETilePathMonitor.h"

void URETilePathMonitorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindButtons();
}

void URETilePathMonitorWidget::NativeDestruct()
{
	UnbindManagerEvents();
	RestoreInput();
	MonitorActor = nullptr;
	CurrentQuestionId = NAME_None;
	CachedRevealedMoves.Reset();
	Super::NativeDestruct();
}

void URETilePathMonitorWidget::InitializeMonitor(ARETilePathMonitor* InMonitorActor)
{
	UnbindManagerEvents();
	MonitorActor = InMonitorActor;
	CurrentQuestionId = NAME_None;
	BindManagerEvents();
	CaptureInput();
	RefreshSessionView();
	RefreshQuestionView();
	RefreshRevealedMovesView();
	ReceiveMonitorInitialized();
}

void URETilePathMonitorWidget::SubmitAnswer(ERETilePathAnswerChoice SubmittedAnswer)
{
	if (IsValid(MonitorActor) == false || AreAllQuestionsSolved() == true)
	{
		return;
	}

	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Manager) == false || Manager->IsSessionStarted() == false)
	{
		RefreshSessionView();
		return;
	}

	const FName SubmitQuestionId = CurrentQuestionId.IsNone() == false ? CurrentQuestionId : MonitorActor->GetCurrentQuestionId();
	if (SubmitQuestionId.IsNone() == true)
	{
		return;
	}

	SetAnswerButtonsEnabled(false);
	MonitorActor->ServerSubmitAnswer(SubmitQuestionId, SubmittedAnswer);
}

void URETilePathMonitorWidget::CloseMonitor()
{
	if (IsValid(MonitorActor) == true)
	{
		ARETilePathManager* Manager = GetTilePathManager();
		if (IsValid(Manager) == true && Manager->IsSessionStarted() == false)
		{
			MonitorActor->ServerCancelGuide();
		}
	}

	RestoreInput();
	RemoveFromParent();
}

void URETilePathMonitorWidget::HandleAnswerResult(bool bCorrect, const FRETilePathRevealedMove& RevealedMove, FName NextQuestionId)
{
	if (IsValid(TXT_Result) == true)
	{
		if (bCorrect == true)
		{
			if (NextQuestionId.IsNone() == true)
			{
				TXT_Result->SetText(AllQuestionsSolvedText);
			}
			else
			{
				const FText DirectionText = URETilePathFunctionLibrary::GetDirectionText(RevealedMove.Direction);
				TXT_Result->SetText(FText::Format(FText::FromString(TEXT("정답 / 다음 방향: {0}")), DirectionText));
			}
		}
		else
		{
			TXT_Result->SetText(FText::FromString(TEXT("오답")));
		}
	}

	if (bCorrect == true && RevealedMove.Direction != ERETilePathDirection::None)
	{
		CachedRevealedMoves.Add(RevealedMove);
	}

	CurrentQuestionId = NextQuestionId;
	RefreshSessionView();
	RefreshQuestionView();
	RefreshRevealedMovesView();
	ReceiveAnswerResult(bCorrect, RevealedMove, NextQuestionId);
}

void URETilePathMonitorWidget::RefreshQuestionView()
{
	FName QuestionId = CurrentQuestionId;
	FRETilePathQuestionRow QuestionRow;
	bool bHasQuestion = false;
	ARETilePathManager* Manager = GetTilePathManager();
	const bool bCanShowQuestion = IsValid(Manager) == true && Manager->IsSessionStarted() == true && AreAllQuestionsSolved() == false;

	if (IsValid(MonitorActor) == true && bCanShowQuestion == true)
	{
		if (QuestionId.IsNone() == false)
		{
			bHasQuestion = MonitorActor->GetQuestionData(QuestionId, QuestionRow);
		}

		if (bHasQuestion == false)
		{
			bHasQuestion = MonitorActor->GetCurrentQuestionData(QuestionId, QuestionRow);
		}
	}

	CurrentQuestionId = bHasQuestion == true ? QuestionId : NAME_None;
	ApplyQuestionToWidgets(CurrentQuestionId, QuestionRow, bHasQuestion);
}

void URETilePathMonitorWidget::RefreshRevealedMovesView()
{
	if (IsValid(MonitorActor) == true)
	{
		MonitorActor->GetRevealedMoves(CachedRevealedMoves);
	}

	if (IsValid(TXT_RevealedMoves) == true)
	{
		TXT_RevealedMoves->SetText(BuildRevealedMovesText());
	}
}

void URETilePathMonitorWidget::RefreshSessionView()
{
	ARETilePathManager* Manager = GetTilePathManager();
	const bool bSessionStarted = IsValid(Manager) == true && Manager->IsSessionStarted() == true;
	const bool bAllQuestionsSolved = bSessionStarted == true && AreAllQuestionsSolved() == true;
	SetQuestionWidgetsVisible(bSessionStarted == true && bAllQuestionsSolved == false);
	SetWaitingWidgetsVisible(!bSessionStarted);
	SetClearWidgetsVisible(bAllQuestionsSolved);
	SetRevealedMovesVisible(bSessionStarted);

	if (IsValid(TXT_WaitingPlayer) == true)
	{
		if (IsValid(Manager) == false)
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("퍼즐 매니저 연결 대기 중")));
		}
		else if (Manager->IsWalkerRegistered() == false)
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("Walker 플레이어가 시작 패널에 상호작용할 때까지 대기 중")));
		}
		else if (Manager->IsGuideRegistered() == false)
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("Display 플레이어 등록 대기 중")));
		}
		else
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("게임 시작 대기 중")));
		}
	}

	if (IsValid(TXT_GameStarted) == true)
	{
		TXT_GameStarted->SetVisibility(bSessionStarted == true && bAllQuestionsSolved == false ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bSessionStarted == true && bAllQuestionsSolved == false)
		{
			TXT_GameStarted->SetText(FText::FromString(TEXT("게임 시작")));
		}
	}
}

ARETilePathMonitor* URETilePathMonitorWidget::GetMonitorActor() const
{
	return MonitorActor;
}

ARETilePathManager* URETilePathMonitorWidget::GetTilePathManager() const
{
	return IsValid(MonitorActor) == true ? MonitorActor->GetTilePathManager() : nullptr;
}

bool URETilePathMonitorWidget::GetCurrentQuestionData(FName& OutQuestionId, FRETilePathQuestionRow& OutQuestionRow) const
{
	OutQuestionId = NAME_None;
	ARETilePathManager* Manager = IsValid(MonitorActor) == true ? MonitorActor->GetTilePathManager() : nullptr;
	return IsValid(Manager) == true && Manager->IsSessionStarted() == true && Manager->AreAllQuestionsSolved() == false && MonitorActor->GetCurrentQuestionData(OutQuestionId, OutQuestionRow) == true;
}

void URETilePathMonitorWidget::GetRevealedMoves(TArray<FRETilePathRevealedMove>& OutRevealedMoves) const
{
	OutRevealedMoves = CachedRevealedMoves;
}

FName URETilePathMonitorWidget::GetCurrentQuestionId() const
{
	return CurrentQuestionId;
}

void URETilePathMonitorWidget::HandleAnswerAClicked()
{
	SubmitAnswer(ERETilePathAnswerChoice::A);
}

void URETilePathMonitorWidget::HandleAnswerBClicked()
{
	SubmitAnswer(ERETilePathAnswerChoice::B);
}

void URETilePathMonitorWidget::HandleAnswerCClicked()
{
	SubmitAnswer(ERETilePathAnswerChoice::C);
}

void URETilePathMonitorWidget::HandleAnswerDClicked()
{
	SubmitAnswer(ERETilePathAnswerChoice::D);
}

void URETilePathMonitorWidget::HandleCloseClicked()
{
	CloseMonitor();
}

void URETilePathMonitorWidget::HandleCancelClicked()
{
	if (IsValid(MonitorActor) == true)
	{
		MonitorActor->ServerCancelGuide();
	}
	RestoreInput();
	RemoveFromParent();
}

void URETilePathMonitorWidget::HandleSessionStateChanged(bool bStarted)
{
	RefreshSessionView();
	RefreshQuestionView();
	RefreshRevealedMovesView();
}

void URETilePathMonitorWidget::HandleParticipantsChanged()
{
	RefreshSessionView();
}

void URETilePathMonitorWidget::BindButtons()
{
	if (IsValid(BTN_A) == true)
	{
		BTN_A->OnClicked.RemoveAll(this);
		BTN_A->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleAnswerAClicked);
	}

	if (IsValid(BTN_B) == true)
	{
		BTN_B->OnClicked.RemoveAll(this);
		BTN_B->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleAnswerBClicked);
	}

	if (IsValid(BTN_C) == true)
	{
		BTN_C->OnClicked.RemoveAll(this);
		BTN_C->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleAnswerCClicked);
	}

	if (IsValid(BTN_D) == true)
	{
		BTN_D->OnClicked.RemoveAll(this);
		BTN_D->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleAnswerDClicked);
	}

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked.RemoveAll(this);
		BTN_Close->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleCloseClicked);
	}

	if (IsValid(BTN_Cancel) == true)
	{
		BTN_Cancel->OnClicked.RemoveAll(this);
		BTN_Cancel->OnClicked.AddDynamic(this, &URETilePathMonitorWidget::HandleCancelClicked);
	}
}

void URETilePathMonitorWidget::ApplyQuestionToWidgets(FName QuestionId, const FRETilePathQuestionRow& QuestionRow, bool bHasQuestion)
{
	const bool bAllQuestionsSolved = AreAllQuestionsSolved();
	if (IsValid(TXT_Question) == true)
	{
		TXT_Question->SetText(bAllQuestionsSolved == true ? FText::GetEmpty() : bHasQuestion == true ? QuestionRow.QuestionText : FText::FromString(TEXT("문제 데이터 없음")));
	}

	SetButtonText(BTN_A, TXT_A, bHasQuestion == true ? QuestionRow.ChoiceA : FText::FromString(TEXT("A")));
	SetButtonText(BTN_B, TXT_B, bHasQuestion == true ? QuestionRow.ChoiceB : FText::FromString(TEXT("B")));
	SetButtonText(BTN_C, TXT_C, bHasQuestion == true ? QuestionRow.ChoiceC : FText::FromString(TEXT("C")));
	SetButtonText(BTN_D, TXT_D, bHasQuestion == true ? QuestionRow.ChoiceD : FText::FromString(TEXT("D")));
	SetAnswerButtonsEnabled(bHasQuestion == true && bAllQuestionsSolved == false);
}

void URETilePathMonitorWidget::SetButtonText(UButton* Button, UTextBlock* ExplicitTextBlock, const FText& Text) const
{
	if (IsValid(ExplicitTextBlock) == true)
	{
		ExplicitTextBlock->SetText(Text);
		return;
	}

	if (IsValid(Button) == false)
	{
		return;
	}

	if (UTextBlock* ButtonText = Cast<UTextBlock>(Button->GetContent()))
	{
		ButtonText->SetText(Text);
	}
}

void URETilePathMonitorWidget::SetAnswerButtonsEnabled(bool bEnabled) const
{
	if (IsValid(BTN_A) == true)
	{
		BTN_A->SetIsEnabled(bEnabled);
	}
	if (IsValid(BTN_B) == true)
	{
		BTN_B->SetIsEnabled(bEnabled);
	}
	if (IsValid(BTN_C) == true)
	{
		BTN_C->SetIsEnabled(bEnabled);
	}
	if (IsValid(BTN_D) == true)
	{
		BTN_D->SetIsEnabled(bEnabled);
	}
}

void URETilePathMonitorWidget::SetQuestionWidgetsVisible(bool bVisible) const
{
	const ESlateVisibility QuestionWidgetsVisibility = bVisible == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (IsValid(TXT_Question) == true)
	{
		TXT_Question->SetVisibility(QuestionWidgetsVisibility);
	}
	if (IsValid(TXT_Result) == true)
	{
		TXT_Result->SetVisibility(QuestionWidgetsVisibility);
	}
	if (IsValid(BTN_A) == true)
	{
		BTN_A->SetVisibility(QuestionWidgetsVisibility);
	}
	if (IsValid(BTN_B) == true)
	{
		BTN_B->SetVisibility(QuestionWidgetsVisibility);
	}
	if (IsValid(BTN_C) == true)
	{
		BTN_C->SetVisibility(QuestionWidgetsVisibility);
	}
	if (IsValid(BTN_D) == true)
	{
		BTN_D->SetVisibility(QuestionWidgetsVisibility);
	}
}

void URETilePathMonitorWidget::SetWaitingWidgetsVisible(bool bVisible) const
{
	const ESlateVisibility WaitingWidgetsVisibility = bVisible == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (IsValid(TXT_WaitingPlayer) == true)
	{
		TXT_WaitingPlayer->SetVisibility(WaitingWidgetsVisibility);
	}
	if (IsValid(BTN_Cancel) == true)
	{
		BTN_Cancel->SetVisibility(WaitingWidgetsVisibility);
	}
}

void URETilePathMonitorWidget::SetClearWidgetsVisible(bool bVisible) const
{
	const ESlateVisibility ClearWidgetsVisibility = bVisible == true ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	if (IsValid(TXT_ClearMessage) == true)
	{
		TXT_ClearMessage->SetVisibility(ClearWidgetsVisibility);
		if (bVisible == true)
		{
			TXT_ClearMessage->SetText(AllQuestionsSolvedText);
		}
		return;
	}

	if (IsValid(TXT_Result) == true && bVisible == true)
	{
		TXT_Result->SetVisibility(ClearWidgetsVisibility);
		TXT_Result->SetText(AllQuestionsSolvedText);
	}
}

void URETilePathMonitorWidget::SetRevealedMovesVisible(bool bVisible) const
{
	if (IsValid(TXT_RevealedMoves) == true)
	{
		TXT_RevealedMoves->SetVisibility(bVisible == true ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

bool URETilePathMonitorWidget::AreAllQuestionsSolved() const
{
	const ARETilePathManager* Manager = GetTilePathManager();
	return IsValid(Manager) == true && Manager->AreAllQuestionsSolved() == true;
}

void URETilePathMonitorWidget::BindManagerEvents()
{
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Manager) == false)
	{
		return;
	}

	Manager->OnTilePathSessionStateChanged.RemoveDynamic(this, &URETilePathMonitorWidget::HandleSessionStateChanged);
	Manager->OnTilePathSessionStateChanged.AddDynamic(this, &URETilePathMonitorWidget::HandleSessionStateChanged);
	Manager->OnTilePathParticipantsChanged.RemoveDynamic(this, &URETilePathMonitorWidget::HandleParticipantsChanged);
	Manager->OnTilePathParticipantsChanged.AddDynamic(this, &URETilePathMonitorWidget::HandleParticipantsChanged);
}

void URETilePathMonitorWidget::UnbindManagerEvents()
{
	ARETilePathManager* Manager = GetTilePathManager();
	if (IsValid(Manager) == false)
	{
		return;
	}

	Manager->OnTilePathSessionStateChanged.RemoveDynamic(this, &URETilePathMonitorWidget::HandleSessionStateChanged);
	Manager->OnTilePathParticipantsChanged.RemoveDynamic(this, &URETilePathMonitorWidget::HandleParticipantsChanged);
}

void URETilePathMonitorWidget::CaptureInput()
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

void URETilePathMonitorWidget::RestoreInput()
{
	APlayerController* PlayerController = CapturedPlayerController.Get();
	if (IsValid(PlayerController) == false || bCapturedInput == false)
	{
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

FText URETilePathMonitorWidget::BuildRevealedMovesText() const
{
	if (CachedRevealedMoves.Num() <= 0)
	{
		return FText::FromString(TEXT("공개된 방향 없음"));
	}

	FString Result;
	for (int32 Index = 0; Index < CachedRevealedMoves.Num(); ++Index)
	{
		const FText DirectionText = URETilePathFunctionLibrary::GetDirectionText(CachedRevealedMoves[Index].Direction);
		Result += FString::Printf(TEXT("%d. %s"), Index + 1, *DirectionText.ToString());
		if (Index < CachedRevealedMoves.Num() - 1)
		{
			Result += TEXT("\n");
		}
	}

	return FText::FromString(Result);
}
