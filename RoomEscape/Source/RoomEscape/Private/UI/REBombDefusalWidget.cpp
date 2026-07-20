#include "UI/REBombDefusalWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Character/REPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "Puzzles/BombDefusal/REBombPatternData.h"
#include "UI/REBombDefusalActionEntryWidget.h"

void UREBombDefusalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//ResolveWidgets();
	//BindCloseButton();
}

void UREBombDefusalWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ResolveWidgets();
	BindCloseButton();
}

void UREBombDefusalWidget::NativeOnDeactivated()
{
	ClearSolvedCloseTimer();
	EndActionRequest();
	UnbindManagerEvents();
	RestoreInput();
	ActionEntries.Reset();
	bFailurePresentationActive = false;
	bFailureResultDelivered = false;
	BombManager = nullptr;

	Super::NativeOnDeactivated();
}

void UREBombDefusalWidget::NativeDestruct()
{
	//ClearSolvedCloseTimer();
	//EndActionRequest();
	//UnbindManagerEvents();
	//RestoreInput();
	//ActionEntries.Reset();
	//bFailurePresentationActive = false;
	//bFailureResultDelivered = false;
	//BombManager = nullptr;
	Super::NativeDestruct();
}

void UREBombDefusalWidget::InitializeBombDefusal(AREBombDefusalManager* InBombManager)
{
	ClearSolvedCloseTimer();
	EndActionRequest();
	UnbindManagerEvents();
	bFailurePresentationActive = false;
	bFailureResultDelivered = false;

	BombManager = InBombManager;
	ResolveWidgets();
	SetFailurePresentationActive(false);
	BindCloseButton();
	BindManagerEvents();
	CaptureInput();
	RebuildActionEntries();
	RefreshFromBombManager();
	SetFeedbackText(FText::GetEmpty(), false);
	ReceiveBombDefusalInitialized(GetPatternData());
}

void UREBombDefusalWidget::RequestCutWire(int32 WireIndex)
{
	if (CanSubmitActions() == false || BombManager->IsWireCut(WireIndex) == true)
	{
		return;
	}

	FREBombWireDefinition WireDefinition;
	if (BombManager->GetWireDefinition(WireIndex, WireDefinition) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] WBP requested an undefined WireIndex. Widget=%s WireIndex=%d"), *GetNameSafe(this), WireIndex);
		return;
	}

	if (AREPlayerController* PlayerController = Cast<AREPlayerController>(GetOwningPlayer()))
	{
		BeginActionRequest();
		PlayerController->ServerSubmitBombWireSelection(BombManager, WireIndex);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] RequestCutWire requires AREPlayerController. Widget=%s"), *GetNameSafe(this));
}

void UREBombDefusalWidget::RequestToggleButton(FName ButtonId)
{
	if (CanSubmitActions() == false || ButtonId.IsNone() == true)
	{
		return;
	}

	FREBombButtonDefinition ButtonDefinition;
	if (BombManager->GetButtonDefinition(ButtonId, ButtonDefinition) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] WBP requested an undefined ButtonId. Widget=%s ButtonId=%s"), *GetNameSafe(this), *ButtonId.ToString());
		return;
	}

	if (AREPlayerController* PlayerController = Cast<AREPlayerController>(GetOwningPlayer()))
	{
		BeginActionRequest();
		PlayerController->ServerSubmitBombButtonSelection(BombManager, ButtonId);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] RequestToggleButton requires AREPlayerController. Widget=%s"), *GetNameSafe(this));
}

void UREBombDefusalWidget::RefreshFromBombManager()
{
	if (IsValid(BombManager) == false)
	{
		ApplyTimerText(0.0f);
		ApplyStepText(0, 0);
		RefreshEntryStates();
		return;
	}

	ApplyTimerText(BombManager->GetRemainingTimeSeconds());
	ApplyStepText(BombManager->GetCurrentStepIndex(), BombManager->GetTotalStepCount());
	RefreshEntryStates();
	ReceiveBombDisplayRefreshed(BombManager->GetRemainingTimeSeconds(), BombManager->GetCurrentStepIndex(), BombManager->GetTotalStepCount());
}

void UREBombDefusalWidget::RebuildActionEntries()
{
	ResolveWidgets();
	ActionEntries.Reset();

	// Fixed-button WBPs opt out so designer-authored children are never removed.
	if (bAutoBuildActionEntries == false)
	{
		return;
	}

	if (IsValid(PNL_Wires) == true)
	{
		PNL_Wires->ClearChildren();
	}
	if (IsValid(PNL_Buttons) == true)
	{
		PNL_Buttons->ClearChildren();
	}

	UREBombPatternData* Pattern = GetPatternData();
	TSubclassOf<UREBombDefusalActionEntryWidget> EffectiveEntryClass = ActionEntryWidgetClass;
	if (EffectiveEntryClass == nullptr)
	{
		EffectiveEntryClass = UREBombDefusalActionEntryWidget::StaticClass();
	}

	if (IsValid(Pattern) == false || EffectiveEntryClass == nullptr)
	{
		return;
	}

	if (IsValid(PNL_Wires) == true)
	{
		for (const FREBombWireDefinition& WireDefinition : Pattern->Wires)
		{
			if (WireDefinition.bShowInWidget == false)
			{
				continue;
			}

			UREBombDefusalActionEntryWidget* Entry = CreateWidget<UREBombDefusalActionEntryWidget>(GetOwningPlayer(), EffectiveEntryClass);
			if (IsValid(Entry) == false)
			{
				continue;
			}

			PNL_Wires->AddChild(Entry);
			Entry->InitializeWireAction(this, WireDefinition);
			ActionEntries.Add(Entry);
		}
	}

	if (IsValid(PNL_Buttons) == true)
	{
		TArray<FREBombButtonDefinition> ResolvedButtons;
		Pattern->GetResolvedButtonDefinitions(ResolvedButtons);
		for (const FREBombButtonDefinition& ButtonDefinition : ResolvedButtons)
		{
			if (ButtonDefinition.bShowInWidget == false)
			{
				continue;
			}

			UREBombDefusalActionEntryWidget* Entry = CreateWidget<UREBombDefusalActionEntryWidget>(GetOwningPlayer(), EffectiveEntryClass);
			if (IsValid(Entry) == false)
			{
				continue;
			}

			PNL_Buttons->AddChild(Entry);
			Entry->InitializeButtonAction(this, ButtonDefinition);
			ActionEntries.Add(Entry);
		}
	}
}

void UREBombDefusalWidget::CloseBombDefusal()
{
	if (IsValid(BombManager) == true)
	{
		BombManager->CloseBombDefusalWidget();
		return;
	}

	RestoreInput();
	if (IsInViewport() == true)
	{
		RemoveFromParent();
	}
}

AREBombDefusalManager* UREBombDefusalWidget::GetBombManager() const
{
	return BombManager;
}

UREBombPatternData* UREBombDefusalWidget::GetPatternData() const
{
	return IsValid(BombManager) == true ? BombManager->GetPatternData() : nullptr;
}

bool UREBombDefusalWidget::CanSubmitActions() const
{
	return bActionRequestPending == false
		&& bFailurePresentationActive == false
		&& IsValid(BombManager) == true
		&& BombManager->IsBombRunning() == true;
}

void UREBombDefusalWidget::HandleCloseClicked()
{
	if (bFailurePresentationActive == false)
	{
		CloseBombDefusal();
	}
}

void UREBombDefusalWidget::HandleTimeChanged(float RemainingTimeSeconds, float TimeLimitSeconds)
{
	ApplyTimerText(RemainingTimeSeconds);
	ReceiveBombDisplayRefreshed(RemainingTimeSeconds, IsValid(BombManager) == true ? BombManager->GetCurrentStepIndex() : 0, IsValid(BombManager) == true ? BombManager->GetTotalStepCount() : 0);
}

void UREBombDefusalWidget::HandleStepChanged(int32 CurrentStepIndex, int32 TotalStepCount)
{
	ApplyStepText(CurrentStepIndex, TotalStepCount);
	RefreshEntryStates();
	ReceiveBombDisplayRefreshed(IsValid(BombManager) == true ? BombManager->GetRemainingTimeSeconds() : 0.0f, CurrentStepIndex, TotalStepCount);
}

void UREBombDefusalWidget::HandleInputResult(AActor* SourceActor, bool bCorrect, FText ResultMessage)
{
	EndActionRequest();
	const bool bShouldDeliverBlueprintResult = bCorrect == true || bFailureResultDelivered == false;
	if (bCorrect == false)
	{
		bFailureResultDelivered = true;
		SetFailurePresentationActive(true);
	}

	SetFeedbackText(ResultMessage, true);
	RefreshFromBombManager();
	if (bShouldDeliverBlueprintResult == true)
	{
		ReceiveBombActionResult(bCorrect, ResultMessage);
	}
}

void UREBombDefusalWidget::HandleElementStatesChanged()
{
	EndActionRequest();
	RefreshEntryStates();
}

void UREBombDefusalWidget::HandlePatternChanged(UREBombPatternData* NewPatternData)
{
	EndActionRequest();
	RebuildActionEntries();
	RefreshFromBombManager();
	ReceiveBombPatternChanged(NewPatternData);
}

void UREBombDefusalWidget::HandlePuzzleStateChanged(EREPuzzleState NewState)
{
	EndActionRequest();
	RefreshFromBombManager();

	if (NewState == EREPuzzleState::Solved)
	{
		ScheduleSolvedClose();
		return;
	}

	if (NewState == EREPuzzleState::Failed)
	{
		// Failure feedback is presented by HandleInputResult/HandleBombExploded.
		// Keep the panel alive and input-disabled until the authoritative reset
		// begins, otherwise TXT_Feedback is removed in the same frame.
		ClearSolvedCloseTimer();
		SetFailurePresentationActive(true);
		return;
	}

	if (NewState == EREPuzzleState::Locked
		|| (NewState == EREPuzzleState::Active && bFailurePresentationActive == true))
	{
		CloseBombDefusal();
	}
}

void UREBombDefusalWidget::HandleBombExploded(AActor* SourceActor, FText ResultMessage)
{
	EndActionRequest();
	SetFailurePresentationActive(true);
	SetFeedbackText(ResultMessage, true);

	// FailBomb normally emits OnBombInputResult immediately before OnBombExploded.
	// Keep this fallback so a future explosion path cannot lose the WBP event,
	// while avoiding duplicate Blueprint feedback animations in the normal path.
	if (bFailureResultDelivered == false)
	{
		bFailureResultDelivered = true;
		ReceiveBombActionResult(false, ResultMessage);
	}
}

void UREBombDefusalWidget::HandleBombRuntimeReset()
{
	ClearSolvedCloseTimer();
	EndActionRequest();
	SetFailurePresentationActive(false);
	CloseBombDefusal();
}

void UREBombDefusalWidget::ResolveWidgets()
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (IsValid(TXT_Timer) == false)
	{
		TXT_Timer = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Timer")));
	}
	if (IsValid(TXT_Step) == false)
	{
		TXT_Step = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Step")));
	}
	if (IsValid(TXT_Feedback) == false)
	{
		TXT_Feedback = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_Feedback")));
	}
	if (IsValid(PNL_Wires) == false)
	{
		PNL_Wires = Cast<UPanelWidget>(WidgetTree->FindWidget(TEXT("PNL_Wires")));
	}
	if (IsValid(PNL_Buttons) == false)
	{
		PNL_Buttons = Cast<UPanelWidget>(WidgetTree->FindWidget(TEXT("PNL_Buttons")));
	}
	if (IsValid(BTN_Close) == false)
	{
		BTN_Close = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Close")));
	}
}

void UREBombDefusalWidget::BindCloseButton()
{
	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->OnClicked.RemoveAll(this);
		BTN_Close->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
}

void UREBombDefusalWidget::BindManagerEvents()
{
	if (IsValid(BombManager) == false)
	{
		return;
	}

	BombManager->OnBombTimeChanged.RemoveDynamic(this, &ThisClass::HandleTimeChanged);
	BombManager->OnBombTimeChanged.AddDynamic(this, &ThisClass::HandleTimeChanged);

	BombManager->OnBombStepChanged.RemoveDynamic(this, &ThisClass::HandleStepChanged);
	BombManager->OnBombStepChanged.AddDynamic(this, &ThisClass::HandleStepChanged);

	BombManager->OnBombInputResult.RemoveDynamic(this, &ThisClass::HandleInputResult);
	BombManager->OnBombInputResult.AddDynamic(this, &ThisClass::HandleInputResult);

	BombManager->OnBombElementStatesChanged.RemoveDynamic(this, &ThisClass::HandleElementStatesChanged);
	BombManager->OnBombElementStatesChanged.AddDynamic(this, &ThisClass::HandleElementStatesChanged);

	BombManager->OnBombPatternChanged.RemoveDynamic(this, &ThisClass::HandlePatternChanged);
	BombManager->OnBombPatternChanged.AddDynamic(this, &ThisClass::HandlePatternChanged);

	BombManager->OnPuzzleStateChanged.RemoveDynamic(this, &ThisClass::HandlePuzzleStateChanged);
	BombManager->OnPuzzleStateChanged.AddDynamic(this, &ThisClass::HandlePuzzleStateChanged);

	BombManager->OnBombExploded.RemoveDynamic(this, &ThisClass::HandleBombExploded);
	BombManager->OnBombExploded.AddDynamic(this, &ThisClass::HandleBombExploded);

	BombManager->OnBombRuntimeReset.RemoveDynamic(this, &ThisClass::HandleBombRuntimeReset);
	BombManager->OnBombRuntimeReset.AddDynamic(this, &ThisClass::HandleBombRuntimeReset);
}

void UREBombDefusalWidget::UnbindManagerEvents()
{
	if (IsValid(BombManager) == false)
	{
		return;
	}

	BombManager->OnBombTimeChanged.RemoveDynamic(this, &ThisClass::HandleTimeChanged);
	BombManager->OnBombStepChanged.RemoveDynamic(this, &ThisClass::HandleStepChanged);
	BombManager->OnBombInputResult.RemoveDynamic(this, &ThisClass::HandleInputResult);
	BombManager->OnBombElementStatesChanged.RemoveDynamic(this, &ThisClass::HandleElementStatesChanged);
	BombManager->OnBombPatternChanged.RemoveDynamic(this, &ThisClass::HandlePatternChanged);
	BombManager->OnPuzzleStateChanged.RemoveDynamic(this, &ThisClass::HandlePuzzleStateChanged);
	BombManager->OnBombExploded.RemoveDynamic(this, &ThisClass::HandleBombExploded);
	BombManager->OnBombRuntimeReset.RemoveDynamic(this, &ThisClass::HandleBombRuntimeReset);
}

void UREBombDefusalWidget::SetFailurePresentationActive(bool bActive)
{
	bFailurePresentationActive = bActive;

	if (IsValid(BTN_Close) == true)
	{
		BTN_Close->SetIsEnabled(bActive == false);
	}

	RefreshEntryStates();
}

void UREBombDefusalWidget::RefreshEntryStates()
{
	for (UREBombDefusalActionEntryWidget* Entry : ActionEntries)
	{
		if (IsValid(Entry) == true)
		{
			Entry->RefreshActionState();
		}
	}
}

void UREBombDefusalWidget::ApplyTimerText(float RemainingTimeSeconds)
{
	if (IsValid(TXT_Timer) == false)
	{
		return;
	}

	const int32 TotalSeconds = FMath::CeilToInt(FMath::Max(RemainingTimeSeconds, 0.0f));
	TXT_Timer->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), TotalSeconds / 60, TotalSeconds % 60)));
}

void UREBombDefusalWidget::ApplyStepText(int32 CurrentStepIndex, int32 TotalStepCount)
{
	if (IsValid(TXT_Step) == false)
	{
		return;
	}

	const int32 DisplayStep = TotalStepCount > 0 ? FMath::Min(CurrentStepIndex + 1, TotalStepCount) : 0;
	TXT_Step->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), DisplayStep, TotalStepCount)));
}

void UREBombDefusalWidget::SetFeedbackText(const FText& FeedbackText, bool bVisible)
{
	if (IsValid(TXT_Feedback) == false)
	{
		return;
	}

	TXT_Feedback->SetText(FeedbackText);
	TXT_Feedback->SetVisibility(bVisible == true ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UREBombDefusalWidget::ScheduleSolvedClose()
{
	if (bCloseWhenSolved == false)
	{
		return;
	}

	ClearSolvedCloseTimer();
	if (SolvedCloseDelay <= 0.0f)
	{
		CloseBombDefusal();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SolvedCloseTimerHandle, this, &ThisClass::CloseBombDefusal, SolvedCloseDelay, false);
	}
}

void UREBombDefusalWidget::ClearSolvedCloseTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SolvedCloseTimerHandle);
	}
}

void UREBombDefusalWidget::BeginActionRequest()
{
	bActionRequestPending = true;
	RefreshEntryStates();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionRequestTimeoutTimerHandle);
		World->GetTimerManager().SetTimer(
			ActionRequestTimeoutTimerHandle,
			this,
			&ThisClass::HandleActionRequestTimeout,
			FMath::Max(ActionRequestTimeoutSeconds, 0.1f),
			false);
	}
}

void UREBombDefusalWidget::EndActionRequest()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActionRequestTimeoutTimerHandle);
	}

	const bool bWasPending = bActionRequestPending;
	bActionRequestPending = false;
	if (bWasPending == true)
	{
		RefreshEntryStates();
	}
}

void UREBombDefusalWidget::HandleActionRequestTimeout()
{
	bActionRequestPending = false;
	RefreshEntryStates();
}

void UREBombDefusalWidget::CaptureInput()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	// SetIgnoreMoveInput / SetIgnoreLookInput are stack based. Reinitializing an
	// already-open widget must not increment the stack a second time, otherwise
	// one close would leave the player permanently input-locked.
	if (bCapturedInput == true)
	{
		return;
	}

	CapturedPlayerController = PlayerController;
	bPreviousMouseCursor = PlayerController->bShowMouseCursor;

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

void UREBombDefusalWidget::RestoreInput()
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

	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
	PlayerController->bShowMouseCursor = bPreviousMouseCursor;
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->FlushPressedKeys();
	CapturedPlayerController.Reset();
	bCapturedInput = false;
}
