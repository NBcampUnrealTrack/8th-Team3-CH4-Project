#include "UI/RETilePathWaitingWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/TilePath/RETilePathManager.h"
#include "Puzzles/TilePath/RETilePathMonitor.h"
#include "Puzzles/TilePath/RETilePathStartPanel.h"

void URETilePathWaitingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(BTN_Cancel) == true)
	{
		BTN_Cancel->OnClicked.RemoveAll(this);
		BTN_Cancel->OnClicked.AddDynamic(this, &URETilePathWaitingWidget::HandleCancelClicked);
	}
}

void URETilePathWaitingWidget::NativeDestruct()
{
	UnbindManagerEvents();
	RestoreInput();
	SourceActor = nullptr;
	TilePathManager = nullptr;
	ParticipantRole = ERETilePathParticipantRole::None;
	Super::NativeDestruct();
}

void URETilePathWaitingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bPulseActive == false)
	{
		return;
	}

	UTextBlock* PulseText = GetPulseTargetText();
	if (IsValid(PulseText) == false)
	{
		bPulseActive = false;
		return;
	}

	PulseElapsedSeconds += InDeltaTime;
	const float Alpha = FMath::Clamp(PulseElapsedSeconds / FMath::Max(PulseDurationSeconds, 0.01f), 0.0f, 1.0f);
	const float Ease = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 3.0f);
	const float StartScale = bPulseGameStart == true ? 1.65f : 1.8f;
	const float CurrentScale = FMath::Lerp(StartScale, 1.0f, Ease);
	PulseText->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
	PulseText->SetRenderOpacity(FMath::Lerp(0.25f, 1.0f, Ease));

	if (Alpha >= 1.0f)
	{
		PulseText->SetRenderScale(FVector2D(1.0f, 1.0f));
		PulseText->SetRenderOpacity(1.0f);
		bPulseActive = false;
	}
}

void URETilePathWaitingWidget::InitializeWaiting(AActor* InSourceActor, ARETilePathManager* InManager, ERETilePathParticipantRole InRole)
{
	UnbindManagerEvents();
	SourceActor = InSourceActor;
	TilePathManager = InManager;
	ParticipantRole = InRole;
	BindManagerEvents();
	CaptureInput();
	RefreshWaitingView();

	if (IsValid(TilePathManager) == true && TilePathManager->GetSessionPhase() == ERETilePathSessionPhase::Countdown)
	{
		StartCountdownPulse(TilePathManager->GetCountdownNumber() <= 0);
	}
}

void URETilePathWaitingWidget::CloseWaiting()
{
	HandleCancelClicked();
}

void URETilePathWaitingWidget::HandleCancelClicked()
{
	if (ParticipantRole == ERETilePathParticipantRole::Guide)
	{
		if (ARETilePathMonitor* Monitor = Cast<ARETilePathMonitor>(SourceActor))
		{
			Monitor->ServerCancelGuide();
		}
	}
	else if (ParticipantRole == ERETilePathParticipantRole::Walker)
	{
		if (ARETilePathStartPanel* StartPanel = Cast<ARETilePathStartPanel>(SourceActor))
		{
			StartPanel->ServerCancelWalker();
		}
	}

	CompleteWaiting(false);
}

void URETilePathWaitingWidget::HandleSessionPhaseChanged(ERETilePathSessionPhase NewPhase)
{
	RefreshWaitingView();

	if (NewPhase == ERETilePathSessionPhase::Playing)
	{
		CompleteWaiting(true);
	}
}

void URETilePathWaitingWidget::HandleCountdownChanged(int32 InCountdownNumber, bool bGameStart)
{
	UpdateCountdownView(InCountdownNumber, bGameStart);
	StartCountdownPulse(bGameStart);
	ReceiveCountdownChanged(InCountdownNumber, bGameStart);
}

void URETilePathWaitingWidget::HandleParticipantsChanged()
{
	RefreshWaitingView();
}

void URETilePathWaitingWidget::BindManagerEvents()
{
	if (IsValid(TilePathManager) == false)
	{
		return;
	}

	TilePathManager->OnTilePathSessionPhaseChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleSessionPhaseChanged);
	TilePathManager->OnTilePathSessionPhaseChanged.AddDynamic(this, &URETilePathWaitingWidget::HandleSessionPhaseChanged);
	TilePathManager->OnTilePathCountdownChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleCountdownChanged);
	TilePathManager->OnTilePathCountdownChanged.AddDynamic(this, &URETilePathWaitingWidget::HandleCountdownChanged);
	TilePathManager->OnTilePathParticipantsChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleParticipantsChanged);
	TilePathManager->OnTilePathParticipantsChanged.AddDynamic(this, &URETilePathWaitingWidget::HandleParticipantsChanged);
}

void URETilePathWaitingWidget::UnbindManagerEvents()
{
	if (IsValid(TilePathManager) == false)
	{
		return;
	}

	TilePathManager->OnTilePathSessionPhaseChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleSessionPhaseChanged);
	TilePathManager->OnTilePathCountdownChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleCountdownChanged);
	TilePathManager->OnTilePathParticipantsChanged.RemoveDynamic(this, &URETilePathWaitingWidget::HandleParticipantsChanged);
}

void URETilePathWaitingWidget::RefreshWaitingView()
{
	const bool bHasManager = IsValid(TilePathManager) == true;
	const ERETilePathSessionPhase Phase = bHasManager == true ? TilePathManager->GetSessionPhase() : ERETilePathSessionPhase::Waiting;
	const bool bCountdown = Phase == ERETilePathSessionPhase::Countdown;
	const bool bPlaying = Phase == ERETilePathSessionPhase::Playing;

	if (IsValid(TXT_WaitingPlayer) == true)
	{
		TXT_WaitingPlayer->SetVisibility(bCountdown == true || bPlaying == true ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

		if (bHasManager == false)
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("퍼즐 매니저 연결 대기 중")));
		}
		else if (ParticipantRole == ERETilePathParticipantRole::Guide)
		{
			TXT_WaitingPlayer->SetText(TilePathManager->IsWalkerRegistered() == true ? FText::FromString(TEXT("게임 시작 준비 중")) : FText::FromString(TEXT("Walker 플레이어가 시작 패널에 상호작용할 때까지 대기 중")));
		}
		else if (ParticipantRole == ERETilePathParticipantRole::Walker)
		{
			TXT_WaitingPlayer->SetText(TilePathManager->IsGuideRegistered() == true ? FText::FromString(TEXT("게임 시작 준비 중")) : FText::FromString(TEXT("Display 플레이어가 모니터에 상호작용할 때까지 대기 중")));
		}
		else
		{
			TXT_WaitingPlayer->SetText(FText::FromString(TEXT("플레이어 대기 중")));
		}
	}

	if (IsValid(BTN_Cancel) == true)
	{
		BTN_Cancel->SetVisibility(bPlaying == true ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		BTN_Cancel->SetIsEnabled(bPlaying == false);
	}

	UpdateCountdownView(bHasManager == true ? TilePathManager->GetCountdownNumber() : -1, bCountdown == true && bHasManager == true && TilePathManager->GetCountdownNumber() <= 0);
}

void URETilePathWaitingWidget::UpdateCountdownView(int32 InCountdownNumber, bool bGameStart)
{
	const bool bShowCountdown = IsValid(TilePathManager) == true && TilePathManager->GetSessionPhase() == ERETilePathSessionPhase::Countdown && bGameStart == false && InCountdownNumber > 0;
	const bool bShowGameStart = IsValid(TilePathManager) == true && TilePathManager->GetSessionPhase() == ERETilePathSessionPhase::Countdown && bGameStart == true;

	if (IsValid(TXT_Countdown) == true)
	{
		TXT_Countdown->SetVisibility(bShowCountdown == true ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShowCountdown == true)
		{
			TXT_Countdown->SetText(FText::AsNumber(InCountdownNumber));
		}
	}

	if (IsValid(TXT_GameStart) == true)
	{
		TXT_GameStart->SetVisibility(bShowGameStart == true ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShowGameStart == true)
		{
			TXT_GameStart->SetText(FText::FromString(TEXT("GAME START")));
		}
	}
}

void URETilePathWaitingWidget::StartCountdownPulse(bool bGameStart)
{
	bPulseGameStart = bGameStart;
	bPulseActive = IsValid(GetPulseTargetText()) == true;
	PulseElapsedSeconds = 0.0f;

	if (UTextBlock* PulseText = GetPulseTargetText())
	{
		const float StartScale = bPulseGameStart == true ? 1.65f : 1.8f;
		PulseText->SetRenderScale(FVector2D(StartScale, StartScale));
		PulseText->SetRenderOpacity(0.25f);
	}
}

UTextBlock* URETilePathWaitingWidget::GetPulseTargetText() const
{
	return bPulseGameStart == true ? TXT_GameStart.Get() : TXT_Countdown.Get();
}

void URETilePathWaitingWidget::CompleteWaiting(bool bStarted)
{
	ReceiveWaitingClosed(bStarted);

	APlayerController* OwningPlayerController = GetOwningPlayer();
	AActor* CachedSourceActor = SourceActor;
	const ERETilePathParticipantRole CachedRole = ParticipantRole;

	RestoreInput();
	RemoveFromParent();

	if (bStarted == true && CachedRole == ERETilePathParticipantRole::Guide)
	{
		if (ARETilePathMonitor* Monitor = Cast<ARETilePathMonitor>(CachedSourceActor))
		{
			Monitor->OpenMonitorWidgetLocal(OwningPlayerController);
		}
	}
}

void URETilePathWaitingWidget::CaptureInput()
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

void URETilePathWaitingWidget::RestoreInput()
{
	APlayerController* PlayerController = CapturedPlayerController.Get();
	if (IsValid(PlayerController) == false || bCapturedInput == false)
	{
		return;
	}

	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
	PlayerController->bShowMouseCursor = bPreviousMouseCursor;
	PlayerController->SetInputMode(FInputModeGameOnly());
	bCapturedInput = false;
}
