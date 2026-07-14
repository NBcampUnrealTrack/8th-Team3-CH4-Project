#include "Puzzles/BombDefusal/REBombDefusalManager.h"

#include "Character/REPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/Components/REBombButtonComponent.h"
#include "Puzzles/BombDefusal/Components/REBombWireComponent.h"
#include "Puzzles/BombDefusal/REBombPatternData.h"
#include "Puzzles/Framework/REPuzzleResetPoint.h"
#include "UI/REBombDefusalWidget.h"
#include "UI/REBombFeedbackWidget.h"
#include "UI/REFadeWidget.h"

AREBombDefusalManager::AREBombDefusalManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bAlwaysRelevant = true;

	DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
	DeviceMesh->SetupAttachment(SceneRoot);
	DeviceMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(SceneRoot);
	InteractionCollision->SetBoxExtent(FVector(120.0, 120.0, 120.0));
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionCollision->SetGenerateOverlapEvents(true);

	TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
	TimerText->SetupAttachment(SceneRoot);
	TimerText->SetHorizontalAlignment(EHTA_Center);
	TimerText->SetVerticalAlignment(EVRTA_TextCenter);
	TimerText->SetRelativeLocation(FVector(0.0, 0.0, 60.0));
	TimerText->SetText(FText::FromString(TEXT("--:--")));

	StepText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StepText"));
	StepText->SetupAttachment(SceneRoot);
	StepText->SetHorizontalAlignment(EHTA_Center);
	StepText->SetVerticalAlignment(EVRTA_TextCenter);
	StepText->SetRelativeLocation(FVector(0.0, 0.0, 40.0));
	StepText->SetText(FText::FromString(TEXT("0/0")));

	InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidgetComponent->SetupAttachment(SceneRoot);
	InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptWidgetComponent->SetGenerateOverlapEvents(false);
	InteractionPromptWidgetComponent->SetVisibility(false);
	InteractionPromptWidgetComponent->SetHiddenInGame(true);
	InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
	InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);
	InteractionPromptWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	InteractionPromptTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InteractionPromptText"));
	InteractionPromptTextRender->SetupAttachment(SceneRoot);
	InteractionPromptTextRender->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptTextRender->SetHorizontalAlignment(EHTA_Center);
	InteractionPromptTextRender->SetVerticalAlignment(EVRTA_TextCenter);
	InteractionPromptTextRender->SetTextRenderColor(FColor::White);
	InteractionPromptTextRender->SetHiddenInGame(true);
	InteractionPromptTextRender->SetVisibility(false);

	InteractionPromptText = FText::FromString(TEXT("폭탄 해제 장치 조작"));
}

void AREBombDefusalManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(InteractionPromptWidgetComponent) == true)
	{
		InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);
		InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
	}

	if (IsValid(InteractionPromptTextRender) == true)
	{
		InteractionPromptTextRender->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptTextRender->SetWorldSize(InteractionPromptWorldSize);
		InteractionPromptTextRender->SetText(InteractionPromptText);
	}

	CacheIntegratedComponents();
	ApplyDefinitionsToIntegratedComponents();
	RefreshNativeDisplay();
}

void AREBombDefusalManager::BeginPlay()
{
	// Cache before Super: AREPuzzleManager::BeginPlay may activate the puzzle and
	// call StartActiveRound during Super::BeginPlay.
	CacheIntegratedComponents();

	if (IsValid(InteractionCollision) == true)
	{
		InteractionCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnInteractionBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnInteractionEndOverlap);
	}

	Super::BeginPlay();

	if (IsValid(InteractionPromptWidgetComponent) == true)
	{
		InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);
		InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
	}

	ApplyDefinitionsToIntegratedComponents();
	ApplyRuntimeStatesToIntegratedComponents();
	ValidateIntegratedConfiguration(HasAuthority());
	RefreshNativeDisplay();
	SynchronizePromptCandidatesFromCurrentOverlaps();
	SetInteractionPromptVisible(false);
	RefreshInteractionPromptVisibility();

	if (HasAuthority() == true && IsActive() == true && RemainingTimeSeconds <= 0.0f)
	{
		StartActiveRound();
	}
}

void AREBombDefusalManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopTimer();
	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(BadEndingFadeWidgetCleanupTimerHandle);

	if (IsValid(InteractionCollision) == true)
	{
		InteractionCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnInteractionBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnInteractionEndOverlap);
	}

	CloseBombDefusalWidget();
	SetInteractionPromptVisible(false);
	LocalPromptCandidates.Reset();
	Super::EndPlay(EndPlayReason);
}

void AREBombDefusalManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PatternData);
	DOREPLIFETIME(ThisClass, CurrentStepIndex);
	DOREPLIFETIME(ThisClass, RemainingTimeSeconds);
	DOREPLIFETIME(ThisClass, CutWireIndices);
	DOREPLIFETIME(ThisClass, PressedButtonIds);
}

void AREBombDefusalManager::SetBadEndingFlowDelegated(bool bDelegated)
{
	if (HasAuthority() == false)
	{
		return;
	}

	bBadEndingFlowDelegated = bDelegated;
}

void AREBombDefusalManager::SetPatternData(UREBombPatternData* InPatternData)
{
	if (HasAuthority() == false)
	{
		return;
	}

	PatternData = InPatternData;
	OnRep_PatternData();
	ForceNetUpdate();

	if (IsActive() == true)
	{
		StartActiveRound();
	}
}

UREBombPatternData* AREBombDefusalManager::GetPatternData() const
{
	return PatternData;
}

int32 AREBombDefusalManager::GetCurrentStepIndex() const
{
	return CurrentStepIndex;
}

int32 AREBombDefusalManager::GetTotalStepCount() const
{
	return IsValid(PatternData) == true ? PatternData->GetStepCount() : 0;
}

float AREBombDefusalManager::GetRemainingTimeSeconds() const
{
	return RemainingTimeSeconds;
}

float AREBombDefusalManager::GetTimeLimitSeconds() const
{
	return IsValid(PatternData) == true ? FMath::Max(PatternData->TimeLimitSeconds, 0.0f) : 0.0f;
}

float AREBombDefusalManager::GetBadEndingResetDelaySeconds() const
{
	constexpr float AbsoluteMinimumFeedbackHoldSeconds = 0.1f;
	return FMath::Max(AbsoluteMinimumFeedbackHoldSeconds, FailureFeedbackHoldSeconds)
		+ FMath::Max(0.0f, BadEndingFadeOutSeconds)
		+ FMath::Max(0.0f, BadEndingBlackHoldSeconds);
}

bool AREBombDefusalManager::IsBombRunning() const
{
	return IsActive() == true
		&& RemainingTimeSeconds > 0.0f
		&& IsValid(PatternData) == true
		&& PatternData->IsPatternValid() == true
		&& CurrentStepIndex < GetTotalStepCount();
}

bool AREBombDefusalManager::GetCurrentStep(FREBombStep& OutStep) const
{
	return IsValid(PatternData) == true && PatternData->GetSolutionStep(CurrentStepIndex, OutStep) == true;
}

bool AREBombDefusalManager::GetWireDefinition(int32 WireIndex, FREBombWireDefinition& OutDefinition) const
{
	return IsValid(PatternData) == true && PatternData->GetWireDefinition(WireIndex, OutDefinition) == true;
}

bool AREBombDefusalManager::GetButtonDefinition(FName ButtonId, FREBombButtonDefinition& OutDefinition) const
{
	return IsValid(PatternData) == true && PatternData->GetButtonDefinition(ButtonId, OutDefinition) == true;
}

bool AREBombDefusalManager::IsWireCut(int32 WireIndex) const
{
	return CutWireIndices.Contains(WireIndex);
}

bool AREBombDefusalManager::IsButtonPressed(FName ButtonId) const
{
	return PressedButtonIds.Contains(ButtonId);
}

bool AREBombDefusalManager::SubmitWireSelectionFromWidget(int32 WireIndex, AActor* Interactor)
{
	return SubmitWireCutByIndex(WireIndex, this, Interactor);
}

bool AREBombDefusalManager::SubmitButtonSelectionFromWidget(FName ButtonId, AActor* Interactor)
{
	return SubmitButtonToggleById(ButtonId, this, Interactor);
}

void AREBombDefusalManager::CloseBombDefusalWidget()
{
	if (IsValid(ActiveBombDefusalWidget) == true)
	{
		ActiveBombDefusalWidget->RemoveFromParent();
		ActiveBombDefusalWidget = nullptr;
	}

	SynchronizePromptCandidatesFromCurrentOverlaps();
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::RefreshIntegratedBombComponents()
{
	CacheIntegratedComponents();
	ApplyDefinitionsToIntegratedComponents();

	if (HasActorBegunPlay() == true)
	{
		ApplyRuntimeStatesToIntegratedComponents();
	}

	ValidateIntegratedConfiguration(true);
}

void AREBombDefusalManager::Interact_Implementation(AActor* Interactor)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BombDefusal] Interaction rejected: target has no authority. Bomb=%s Interactor=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Interactor));
		return;
	}

	if (CanInteractWithBomb(Interactor) == false)
	{
		const bool bPatternAssigned = IsValid(PatternData) == true;
		const bool bPatternValid = bPatternAssigned == true && PatternData->IsPatternValid() == true;
		const bool bWidgetAssigned = BombDefusalWidgetClass != nullptr;
		const bool bInteractorValid = IsValid(Interactor) == true;
		const bool bInteractorInRange = bInteractorValid == true && CanSubmitInputFromInteractor(Interactor) == true;
		const float Distance = bInteractorValid == true
			? FVector::Dist(Interactor->GetActorLocation(), GetActorLocation())
			: -1.0f;

		UE_LOG(LogTemp, Warning,
			TEXT("[BombDefusal] Interaction rejected. Bomb=%s Interactor=%s State=%d Active=%s PatternAssigned=%s PatternValid=%s WidgetAssigned=%s InRange=%s Distance=%.1f MaxDistance=%.1f. Visual Wire/Button mapping warnings do not block interaction."),
			*GetNameSafe(this),
			*GetNameSafe(Interactor),
			static_cast<int32>(GetPuzzleState()),
			IsActive() ? TEXT("true") : TEXT("false"),
			bPatternAssigned ? TEXT("true") : TEXT("false"),
			bPatternValid ? TEXT("true") : TEXT("false"),
			bWidgetAssigned ? TEXT("true") : TEXT("false"),
			bInteractorInRange ? TEXT("true") : TEXT("false"),
			Distance,
			MaxWidgetInteractionDistance);
		return;
	}

	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	if (IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] Could not resolve PlayerController. Bomb=%s Interactor=%s"), *GetNameSafe(this), *GetNameSafe(Interactor));
		return;
	}

	if (PlayerController->IsLocalController() == true)
	{
		OpenBombDefusalWidgetForLocalPlayer(PlayerController);
		return;
	}

	if (AREPlayerController* REPlayerController = Cast<AREPlayerController>(PlayerController))
	{
		REPlayerController->ClientOpenBombDefusalWidget(this);
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[BombDefusal] Remote WBP open requires AREPlayerController. Bomb=%s Controller=%s"),
		*GetNameSafe(this), *GetNameSafe(PlayerController));
}

bool AREBombDefusalManager::CanInteractWithBomb(AActor* Interactor) const
{
	// PatternData owns gameplay and WBP actions. Integrated Wire/Button
	// components are presentation bindings only, so an incomplete visual setup
	// must never suppress the interaction UI.
	return IsValid(Interactor) == true
		&& IsActive() == true
		&& IsValid(PatternData) == true
		&& PatternData->IsPatternValid() == true
		&& BombDefusalWidgetClass != nullptr
		&& CanSubmitInputFromInteractor(Interactor) == true;
}

bool AREBombDefusalManager::CanActivatePuzzle() const
{
	return Super::CanActivatePuzzle() == true
		&& IsValid(PatternData) == true
		&& PatternData->IsPatternValid() == true;
}

void AREBombDefusalManager::HandlePuzzleActivated()
{
	StartActiveRound();
}

void AREBombDefusalManager::HandlePuzzleLocked()
{
	StopTimer();
	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	ResetRuntimeState();
	CloseBombDefusalWidget();
}

void AREBombDefusalManager::HandlePuzzleSolved()
{
	StopTimer();
	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	RefreshNativeDisplay();
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::HandlePuzzleFailed()
{
	StopTimer();
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::HandleSavedSolvedStateRestored()
{
	SetCurrentStepIndex(GetTotalStepCount());
	SetRemainingTimeSeconds(0.0f);
	HandlePuzzleSolved();
	OnPuzzleSolved.Broadcast();
}

void AREBombDefusalManager::OnRep_State()
{
	Super::OnRep_State();

	// A reset can arrive through replicated state before the reliable runtime
	// reset multicast. Cancel any client-local delayed fade so it cannot start
	// after the bomb has already returned to Locked/Active.
	if (IsLocked() == true || IsActive() == true)
	{
		GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	}

	SynchronizePromptCandidatesFromCurrentOverlaps();
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::OnRep_PatternData()
{
	CacheIntegratedComponents();
	ApplyDefinitionsToIntegratedComponents();
	ApplyRuntimeStatesToIntegratedComponents();
	RefreshNativeDisplay();
	SynchronizePromptCandidatesFromCurrentOverlaps();
	RefreshInteractionPromptVisibility();
	OnBombPatternChanged.Broadcast(PatternData);
	ReceiveBombPatternChanged(PatternData);
}

void AREBombDefusalManager::OnRep_CurrentStepIndex()
{
	RefreshNativeDisplay();
	OnBombStepChanged.Broadcast(CurrentStepIndex, GetTotalStepCount());
}

void AREBombDefusalManager::OnRep_RemainingTimeSeconds()
{
	RefreshNativeDisplay();
	OnBombTimeChanged.Broadcast(RemainingTimeSeconds, GetTimeLimitSeconds());
}

void AREBombDefusalManager::OnRep_RuntimeElementStates()
{
	ApplyRuntimeStatesToIntegratedComponents();
	OnBombElementStatesChanged.Broadcast();
	ReceiveBombElementStatesChanged();
}

void AREBombDefusalManager::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldTrackPromptActor(OtherActor) == false)
	{
		return;
	}

	for (const TWeakObjectPtr<AActor>& Candidate : LocalPromptCandidates)
	{
		if (Candidate.Get() == OtherActor)
		{
			RefreshInteractionPromptVisibility();
			return;
		}
	}

	LocalPromptCandidates.Add(OtherActor);
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	LocalPromptCandidates.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Candidate)
	{
		return Candidate.IsValid() == false || Candidate.Get() == OtherActor;
	});
	RefreshInteractionPromptVisibility();
}

void AREBombDefusalManager::MulticastBombInputResult_Implementation(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage)
{
	OnBombInputResult.Broadcast(SourceActor, bCorrect, ResultMessage);
	ReceiveBombInputResult(SourceActor, bCorrect, ResultMessage);
	ShowFeedbackWidgetLocal(SourceActor, TargetPlayerState, bCorrect, ResultMessage);
}

void AREBombDefusalManager::MulticastBombExploded_Implementation(AActor* SourceActor, const FText& ResultMessage)
{
	OnBombExploded.Broadcast(SourceActor, ResultMessage);
	ReceiveBombExploded(SourceActor, ResultMessage);
	ScheduleBadEndingFadeOutLocal();
}

void AREBombDefusalManager::MulticastBombRuntimeReset_Implementation()
{
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	OnBombRuntimeReset.Broadcast();
	ReceiveBombRuntimeReset();
}

void AREBombDefusalManager::MulticastBombCheckpointRestored_Implementation()
{
	OnBombCheckpointRestored.Broadcast();
	ReceiveBombCheckpointRestored();
	PlayBadEndingFadeInLocal();
}

void AREBombDefusalManager::StartActiveRound()
{
	if (HasAuthority() == false
		|| IsValid(PatternData) == false
		|| PatternData->IsPatternValid() == false)
	{
		StopTimer();
		return;
	}

	// Visual mapping problems are diagnostics only. They must not turn a
	// presentation issue into a locked puzzle or inaccessible WBP.
	ValidateIntegratedConfiguration(true);

	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);
	ResetRuntimeState();
	StartTimer();

	if (bPendingCheckpointRestoreNotify == true)
	{
		bPendingCheckpointRestoreNotify = false;
		MulticastBombRuntimeReset();
		MulticastBombCheckpointRestored();
	}
}

void AREBombDefusalManager::ResetRuntimeState()
{
	SetCurrentStepIndex(0);
	SetRemainingTimeSeconds(GetTimeLimitSeconds());
	ResetRuntimeElementStatesFromPattern();
	RefreshNativeDisplay();
}

void AREBombDefusalManager::ResetRuntimeElementStatesFromPattern()
{
	if (HasAuthority() == false)
	{
		return;
	}

	CutWireIndices.Reset();
	PressedButtonIds.Reset();

	if (IsValid(PatternData) == true)
	{
		TArray<FREBombButtonDefinition> ResolvedButtons;
		PatternData->GetResolvedButtonDefinitions(ResolvedButtons);
		for (const FREBombButtonDefinition& ButtonDefinition : ResolvedButtons)
		{
			if (ButtonDefinition.bInitialPressed == true && ButtonDefinition.ButtonId.IsNone() == false)
			{
				PressedButtonIds.AddUnique(ButtonDefinition.ButtonId);
			}
		}
	}

	PressedButtonIds.Sort([](const FName& A, const FName& B)
	{
		return A.ToString() < B.ToString();
	});

	OnRep_RuntimeElementStates();
	ForceNetUpdate();
}

void AREBombDefusalManager::ApplyDefinitionsToIntegratedComponents()
{
	for (UREBombWireComponent* WireComponent : IntegratedWireComponents)
	{
		if (IsValid(WireComponent) == false)
		{
			continue;
		}

		FREBombWireDefinition WireDefinition;
		if (GetWireDefinition(WireComponent->GetWireIndex(), WireDefinition) == true)
		{
			WireComponent->ApplyDefinition(WireDefinition);
		}
	}

	for (UREBombButtonComponent* ButtonComponent : IntegratedButtonComponents)
	{
		if (IsValid(ButtonComponent) == false)
		{
			continue;
		}

		FREBombButtonDefinition ButtonDefinition;
		if (GetButtonDefinition(ButtonComponent->GetButtonId(), ButtonDefinition) == true)
		{
			ButtonComponent->ApplyDefinition(ButtonDefinition);
		}
	}
}

void AREBombDefusalManager::ApplyRuntimeStatesToIntegratedComponents()
{
	if (HasActorBegunPlay() == false)
	{
		return;
	}

	// Ambiguous visual keys stay in their safe default state. This keeps a
	// duplicated WireIndex/ButtonId from applying one logical state to several
	// physical meshes, while the DA-driven WBP remains usable.
	TMap<int32, int32> WireIndexCounts;
	for (const UREBombWireComponent* WireComponent : IntegratedWireComponents)
	{
		if (IsValid(WireComponent) == true)
		{
			++WireIndexCounts.FindOrAdd(WireComponent->GetWireIndex());
		}
	}

	for (UREBombWireComponent* WireComponent : IntegratedWireComponents)
	{
		if (IsValid(WireComponent) == false)
		{
			continue;
		}

		const int32 WireIndex = WireComponent->GetWireIndex();
		FREBombWireDefinition WireDefinition;
		const int32* MappingCount = WireIndexCounts.Find(WireIndex);
		const bool bHasUniqueValidMapping = WireIndex != INDEX_NONE
			&& MappingCount != nullptr
			&& *MappingCount == 1
			&& GetWireDefinition(WireIndex, WireDefinition) == true;

		WireComponent->ApplyCutState(bHasUniqueValidMapping == true && IsWireCut(WireIndex) == true);
	}

	TMap<FName, int32> ButtonIdCounts;
	for (const UREBombButtonComponent* ButtonComponent : IntegratedButtonComponents)
	{
		if (IsValid(ButtonComponent) == true)
		{
			++ButtonIdCounts.FindOrAdd(ButtonComponent->GetButtonId());
		}
	}

	for (UREBombButtonComponent* ButtonComponent : IntegratedButtonComponents)
	{
		if (IsValid(ButtonComponent) == false)
		{
			continue;
		}

		const FName ButtonId = ButtonComponent->GetButtonId();
		FREBombButtonDefinition ButtonDefinition;
		const int32* MappingCount = ButtonIdCounts.Find(ButtonId);
		const bool bHasUniqueValidMapping = ButtonId.IsNone() == false
			&& MappingCount != nullptr
			&& *MappingCount == 1
			&& GetButtonDefinition(ButtonId, ButtonDefinition) == true;

		ButtonComponent->ApplyPressedState(bHasUniqueValidMapping == true && IsButtonPressed(ButtonId) == true);
	}
}

void AREBombDefusalManager::CacheIntegratedComponents()
{
	IntegratedWireComponents.Reset();
	IntegratedButtonComponents.Reset();

	TArray<UREBombWireComponent*> FoundWires;
	GetComponents<UREBombWireComponent>(FoundWires);
	for (UREBombWireComponent* WireComponent : FoundWires)
	{
		if (IsValid(WireComponent) == true)
		{
			IntegratedWireComponents.Add(WireComponent);
		}
	}

	TArray<UREBombButtonComponent*> FoundButtons;
	GetComponents<UREBombButtonComponent>(FoundButtons);
	for (UREBombButtonComponent* ButtonComponent : FoundButtons)
	{
		if (IsValid(ButtonComponent) == true)
		{
			IntegratedButtonComponents.Add(ButtonComponent);
		}
	}
}

void AREBombDefusalManager::StartTimer()
{
	StopTimer();
	SetRemainingTimeSeconds(GetTimeLimitSeconds());
	GetWorldTimerManager().SetTimer(BombTimerHandle, this, &ThisClass::TickTimer, 0.1f, true);
}

void AREBombDefusalManager::StopTimer()
{
	GetWorldTimerManager().ClearTimer(BombTimerHandle);
}

void AREBombDefusalManager::TickTimer()
{
	if (HasAuthority() == false || IsActive() == false)
	{
		StopTimer();
		return;
	}

	const float NewRemainingTime = FMath::Max(RemainingTimeSeconds - 0.1f, 0.0f);
	SetRemainingTimeSeconds(NewRemainingTime);
	if (NewRemainingTime <= 0.0f)
	{
		FailBomb(this, nullptr, BuildFailureFeedbackMessage());
	}
}

void AREBombDefusalManager::SetCurrentStepIndex(int32 NewStepIndex)
{
	const int32 ClampedStepIndex = FMath::Clamp(NewStepIndex, 0, GetTotalStepCount());
	if (CurrentStepIndex == ClampedStepIndex)
	{
		RefreshNativeDisplay();
		return;
	}

	CurrentStepIndex = ClampedStepIndex;
	OnRep_CurrentStepIndex();
	ForceNetUpdate();
}

void AREBombDefusalManager::SetRemainingTimeSeconds(float NewRemainingTimeSeconds)
{
	const float ClampedRemainingTime = FMath::Max(NewRemainingTimeSeconds, 0.0f);
	if (FMath::IsNearlyEqual(RemainingTimeSeconds, ClampedRemainingTime, 0.01f) == true)
	{
		RefreshNativeDisplay();
		return;
	}

	RemainingTimeSeconds = ClampedRemainingTime;
	OnRep_RemainingTimeSeconds();
	ForceNetUpdate();
}

void AREBombDefusalManager::SetWireCutState(int32 WireIndex, bool bNewCut)
{
	if (HasAuthority() == false)
	{
		return;
	}

	const bool bWasCut = IsWireCut(WireIndex);
	if (bWasCut == bNewCut)
	{
		return;
	}

	if (bNewCut == true)
	{
		CutWireIndices.AddUnique(WireIndex);
		CutWireIndices.Sort();
	}
	else
	{
		CutWireIndices.Remove(WireIndex);
	}

	OnRep_RuntimeElementStates();
	ForceNetUpdate();
}

void AREBombDefusalManager::SetButtonPressedState(FName ButtonId, bool bNewPressed)
{
	if (HasAuthority() == false || ButtonId.IsNone() == true)
	{
		return;
	}

	const bool bWasPressed = IsButtonPressed(ButtonId);
	if (bWasPressed == bNewPressed)
	{
		return;
	}

	if (bNewPressed == true)
	{
		PressedButtonIds.AddUnique(ButtonId);
		PressedButtonIds.Sort([](const FName& A, const FName& B)
		{
			return A.ToString() < B.ToString();
		});
	}
	else
	{
		PressedButtonIds.Remove(ButtonId);
	}

	OnRep_RuntimeElementStates();
	ForceNetUpdate();
}

bool AREBombDefusalManager::SubmitWireCutByIndex(int32 WireIndex, AActor* SourceActor, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || CanSubmitInputFromInteractor(Interactor) == false)
	{
		return false;
	}

	FREBombWireDefinition WireDefinition;
	if (GetWireDefinition(WireIndex, WireDefinition) == false)
	{
		return false;
	}

	FText FailureMessage;
	if (ValidateCurrentWireStep(WireIndex, FailureMessage) == false)
	{
		FailBomb(IsValid(SourceActor) == true ? SourceActor : this, Interactor, FailureMessage);
		return false;
	}

	SetWireCutState(WireIndex, true);
	AdvanceStep(IsValid(SourceActor) == true ? SourceActor : this, Interactor, BuildSuccessFeedbackMessage());
	return true;
}

bool AREBombDefusalManager::SubmitButtonToggleById(FName ButtonId, AActor* SourceActor, AActor* Interactor)
{
	if (HasAuthority() == false || CanAcceptInput() == false || CanSubmitInputFromInteractor(Interactor) == false)
	{
		return false;
	}

	FREBombButtonDefinition ButtonDefinition;
	if (GetButtonDefinition(ButtonId, ButtonDefinition) == false)
	{
		return false;
	}

	const bool bNextPressed = IsButtonPressed(ButtonId) == false;

	FText FailureMessage;
	if (ValidateCurrentButtonStep(ButtonId, bNextPressed, FailureMessage) == false)
	{
		FailBomb(IsValid(SourceActor) == true ? SourceActor : this, Interactor, FailureMessage);
		return false;
	}

	SetButtonPressedState(ButtonId, bNextPressed);
	AdvanceStep(IsValid(SourceActor) == true ? SourceActor : this, Interactor, BuildSuccessFeedbackMessage());
	return true;
}

void AREBombDefusalManager::AdvanceStep(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage)
{
	APlayerState* TargetPlayerState = ResolvePlayerState(Interactor);
	MulticastBombInputResult(SourceActor, TargetPlayerState, true, ResultMessage);
	const int32 NextStepIndex = CurrentStepIndex + 1;
	if (NextStepIndex >= GetTotalStepCount())
	{
		CompleteBomb(SourceActor, Interactor);
		return;
	}

	SetCurrentStepIndex(NextStepIndex);
}

void AREBombDefusalManager::CompleteBomb(AActor* SourceActor, AActor* Interactor)
{
	SetCurrentStepIndex(GetTotalStepCount());
	MarkSolved();
	MulticastBombInputResult(SourceActor, ResolvePlayerState(Interactor), true, BuildSolvedFeedbackMessage());
}

void AREBombDefusalManager::FailBomb(AActor* SourceActor, AActor* Interactor, const FText& ResultMessage)
{
	if (HasAuthority() == false || IsSolved() == true || IsFailed() == true)
	{
		return;
	}

	SetRemainingTimeSeconds(0.0f);
	const FText FailureFeedbackMessage = ResultMessage;
	MulticastBombInputResult(SourceActor, ResolvePlayerState(Interactor), false, FailureFeedbackMessage);
	MarkFailed();
	MulticastBombExploded(SourceActor, FailureFeedbackMessage);

	GetWorldTimerManager().ClearTimer(BadEndingResetTimerHandle);

	if (bBadEndingFlowDelegated == true)
	{
		bPendingCheckpointRestoreNotify = true;
		return;
	}

	GetWorldTimerManager().SetTimer(BadEndingResetTimerHandle, this, &ThisClass::ResolveBadEnding, GetBadEndingResetDelaySeconds(), false);
}

void AREBombDefusalManager::ResolveBadEnding()
{
	if (HasAuthority() == false || IsSolved() == true)
	{
		return;
	}

	TeleportPlayersToBadEndingCheckpoints();
	ResetRuntimeState();
	SetPuzzleState(EREPuzzleState::Active);
	StartTimer();
	MulticastBombRuntimeReset();
	MulticastBombCheckpointRestored();
}

void AREBombDefusalManager::TeleportPlayersToBadEndingCheckpoints()
{
	TArray<AREPuzzleResetPoint*> CheckpointPoints;
	ResolveBadEndingCheckpointPoints(CheckpointPoints);
	if (CheckpointPoints.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] Bad ending checkpoint is not configured. Manager=%s CheckpointId=%s"), *GetName(), *BadEndingCheckpointId.ToString());
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false)
		{
			continue;
		}

		APawn* Pawn = PlayerController->GetPawn();
		if (IsValid(Pawn) == false)
		{
			continue;
		}

		AREPuzzleResetPoint* ResetPoint = CheckpointPoints.IsValidIndex(PlayerIndex) == true ? CheckpointPoints[PlayerIndex] : CheckpointPoints[0];
		if (IsValid(ResetPoint) == false)
		{
			continue;
		}

		const FTransform CheckpointTransform = ResetPoint->GetCheckpointTransform();
		const FRotator CheckpointRotation = CheckpointTransform.GetRotation().Rotator();
		Pawn->TeleportTo(CheckpointTransform.GetLocation(), CheckpointRotation, false, true);
		PlayerController->SetControlRotation(CheckpointRotation);
		++PlayerIndex;
	}
}

void AREBombDefusalManager::ResolveBadEndingCheckpointPoints(TArray<AREPuzzleResetPoint*>& OutCheckpointPoints) const
{
	OutCheckpointPoints.Reset();

	for (const TObjectPtr<AREPuzzleResetPoint>& ResetPointPtr : BadEndingCheckpointPoints)
	{
		AREPuzzleResetPoint* ResetPoint = ResetPointPtr.Get();
		if (IsValid(ResetPoint) == true && ResetPoint->CanUseAsCheckpoint() == true)
		{
			OutCheckpointPoints.AddUnique(ResetPoint);
		}
	}

	if (OutCheckpointPoints.Num() <= 0 && BadEndingCheckpointId.IsNone() == false)
	{
		TArray<AActor*> ResetPointActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AREPuzzleResetPoint::StaticClass(), ResetPointActors);
		for (AActor* ResetPointActor : ResetPointActors)
		{
			AREPuzzleResetPoint* ResetPoint = Cast<AREPuzzleResetPoint>(ResetPointActor);
			if (IsValid(ResetPoint) == true && ResetPoint->CanUseAsCheckpoint() == true && ResetPoint->GetCheckpointId() == BadEndingCheckpointId)
			{
				OutCheckpointPoints.AddUnique(ResetPoint);
			}
		}
	}

	OutCheckpointPoints.Sort([](const AREPuzzleResetPoint& A, const AREPuzzleResetPoint& B)
	{
		if (A.GetCheckpointOrder() == B.GetCheckpointOrder())
		{
			return A.GetName() < B.GetName();
		}
		return A.GetCheckpointOrder() < B.GetCheckpointOrder();
	});
}

void AREBombDefusalManager::ScheduleBadEndingFadeOutLocal()
{
	GetWorldTimerManager().ClearTimer(BadEndingFadeOutDelayTimerHandle);

	constexpr float AbsoluteMinimumFeedbackHoldSeconds = 0.1f;
	const float HoldSeconds = FMath::Max(AbsoluteMinimumFeedbackHoldSeconds, FailureFeedbackHoldSeconds);
	GetWorldTimerManager().SetTimer(
		BadEndingFadeOutDelayTimerHandle,
		this,
		&ThisClass::PlayBadEndingFadeOutLocal,
		HoldSeconds,
		false);
}

void AREBombDefusalManager::PlayBadEndingFadeOutLocal()
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(nullptr);
	if (IsValid(LocalPlayerController) == false || BadEndingFadeWidgetClass == nullptr)
	{
		return;
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false || ActiveBadEndingFadeWidget->GetClass() != BadEndingFadeWidgetClass.Get())
	{
		ActiveBadEndingFadeWidget = CreateWidget<UREFadeWidget>(LocalPlayerController, BadEndingFadeWidgetClass);
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false)
	{
		return;
	}

	if (ActiveBadEndingFadeWidget->IsInViewport() == false)
	{
		ActiveBadEndingFadeWidget->AddToViewport(BadEndingFadeWidgetZOrder);
	}

	GetWorldTimerManager().ClearTimer(BadEndingFadeWidgetCleanupTimerHandle);
	ActiveBadEndingFadeWidget->PlayFadeOut(BadEndingFadeOutSeconds);
}

void AREBombDefusalManager::PlayBadEndingFadeInLocal()
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(nullptr);
	if (IsValid(LocalPlayerController) == false || BadEndingFadeWidgetClass == nullptr)
	{
		return;
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false || ActiveBadEndingFadeWidget->GetClass() != BadEndingFadeWidgetClass.Get())
	{
		ActiveBadEndingFadeWidget = CreateWidget<UREFadeWidget>(LocalPlayerController, BadEndingFadeWidgetClass);
	}

	if (IsValid(ActiveBadEndingFadeWidget) == false)
	{
		return;
	}

	if (ActiveBadEndingFadeWidget->IsInViewport() == false)
	{
		ActiveBadEndingFadeWidget->AddToViewport(BadEndingFadeWidgetZOrder);
	}

	ActiveBadEndingFadeWidget->PlayFadeIn(BadEndingFadeInSeconds);
	GetWorldTimerManager().ClearTimer(BadEndingFadeWidgetCleanupTimerHandle);
	GetWorldTimerManager().SetTimer(BadEndingFadeWidgetCleanupTimerHandle, this, &ThisClass::CleanupBadEndingFadeWidgetLocal, FMath::Max(BadEndingFadeInSeconds, 0.0f) + 0.05f, false);
}

void AREBombDefusalManager::CleanupBadEndingFadeWidgetLocal()
{
	if (IsValid(ActiveBadEndingFadeWidget) == true)
	{
		ActiveBadEndingFadeWidget->RemoveFromParent();
		ActiveBadEndingFadeWidget = nullptr;
	}
}

void AREBombDefusalManager::ShowFeedbackWidgetLocal(AActor* SourceActor, APlayerState* TargetPlayerState, bool bCorrect, const FText& ResultMessage)
{
	if (bShowStandaloneFeedbackWhileBombWidgetOpen == false
		&& IsValid(ActiveBombDefusalWidget) == true
		&& ActiveBombDefusalWidget->IsInViewport() == true)
	{
		return;
	}

	APlayerController* LocalPlayerController = ResolveLocalPlayerController(TargetPlayerState);
	if (IsValid(LocalPlayerController) == false || FeedbackWidgetClass == nullptr)
	{
		return;
	}

	if (IsValid(ActiveFeedbackWidget) == false || ActiveFeedbackWidget->GetClass() != FeedbackWidgetClass.Get())
	{
		ActiveFeedbackWidget = CreateWidget<UREBombFeedbackWidget>(LocalPlayerController, FeedbackWidgetClass);
	}

	if (IsValid(ActiveFeedbackWidget) == false)
	{
		return;
	}

	if (ActiveFeedbackWidget->IsInViewport() == false)
	{
		ActiveFeedbackWidget->AddToViewport(80);
	}

	ActiveFeedbackWidget->InitializeFeedback(SourceActor, bCorrect, ResultMessage, FeedbackWidgetDisplaySeconds);
}

void AREBombDefusalManager::OpenBombDefusalWidgetForLocalPlayer(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || BombDefusalWidgetClass == nullptr)
	{
		return;
	}

	if (IsValid(ActiveBombDefusalWidget) == false || ActiveBombDefusalWidget->GetClass() != BombDefusalWidgetClass.Get())
	{
		ActiveBombDefusalWidget = CreateWidget<UREBombDefusalWidget>(PlayerController, BombDefusalWidgetClass);
	}

	if (IsValid(ActiveBombDefusalWidget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] Failed to create bomb widget. Bomb=%s WidgetClass=%s"), *GetNameSafe(this), *GetNameSafe(BombDefusalWidgetClass.Get()));
		return;
	}

	if (ActiveBombDefusalWidget->IsInViewport() == false)
	{
		ActiveBombDefusalWidget->AddToViewport(BombDefusalWidgetZOrder);
	}

	ActiveBombDefusalWidget->InitializeBombDefusal(this);
	RefreshInteractionPromptVisibility();
}

APlayerController* AREBombDefusalManager::ResolvePlayerController(AActor* Actor) const
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

APlayerController* AREBombDefusalManager::ResolveLocalPlayerController(APlayerState* TargetPlayerState) const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
		{
			continue;
		}

		if (IsValid(TargetPlayerState) == false || PlayerController->PlayerState == TargetPlayerState)
		{
			return PlayerController;
		}
	}

	return nullptr;
}

FText AREBombDefusalManager::BuildSuccessFeedbackMessage() const
{
	return FText::FromString(TEXT("장치 조작이 입력되었습니다."));
}

FText AREBombDefusalManager::BuildSolvedFeedbackMessage() const
{
	return FText::FromString(TEXT("폭탄 해제가 완료되었습니다."));
}

FText AREBombDefusalManager::BuildFailureFeedbackMessage() const
{
	return FText::FromString(TEXT("폭탄이 폭발했습니다. 체크포인트에서 다시 시작합니다."));
}

bool AREBombDefusalManager::CanAcceptInput() const
{
	return IsActive() == true
		&& IsValid(PatternData) == true
		&& PatternData->IsPatternValid() == true
		&& CurrentStepIndex < GetTotalStepCount();
}

bool AREBombDefusalManager::CanSubmitInputFromInteractor(AActor* Interactor) const
{
	if (IsValid(Interactor) == false)
	{
		return false;
	}

	if (MaxWidgetInteractionDistance <= 0.0f)
	{
		return true;
	}

	return FVector::DistSquared(Interactor->GetActorLocation(), GetActorLocation())
		<= FMath::Square(MaxWidgetInteractionDistance);
}

bool AREBombDefusalManager::ValidateIntegratedConfiguration(bool bLogWarnings) const
{
	bool bValid = true;

	if (IsValid(PatternData) == false)
	{
		if (bLogWarnings == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] PatternData is not assigned. Bomb=%s"), *GetNameSafe(this));
		}
		return false;
	}

	if (PatternData->IsPatternValid() == false)
	{
		bValid = false;
		if (bLogWarnings == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] PatternData is invalid. Check duplicate IDs and the solution sequence. Bomb=%s Data=%s"), *GetNameSafe(this), *GetNameSafe(PatternData.Get()));
		}
	}

	TSet<int32> ComponentWireIndices;
	TMap<int32, const UREBombWireComponent*> FirstWireComponentByIndex;
	for (const UREBombWireComponent* WireComponent : IntegratedWireComponents)
	{
		if (IsValid(WireComponent) == false)
		{
			continue;
		}

		const int32 WireIndex = WireComponent->GetWireIndex();
		FREBombWireDefinition WireDefinition;
		const bool bHasDefinition = WireIndex != INDEX_NONE
			&& PatternData->GetWireDefinition(WireIndex, WireDefinition);
		const UREBombWireComponent* const* ExistingWireComponent = FirstWireComponentByIndex.Find(WireIndex);
		const bool bDuplicate = ExistingWireComponent != nullptr;
		const bool bMappingValid = WireIndex != INDEX_NONE
			&& bHasDefinition == true
			&& bDuplicate == false;

		if (bMappingValid == false)
		{
			bValid = false;
			if (bLogWarnings == true)
			{
				if (bDuplicate == true)
				{
					UE_LOG(LogTemp, Error,
						TEXT("[BombDefusal] Duplicate WireIndex=%d. Components '%s' and '%s' point to the same logical wire. Give every physical wire a unique index matching DA.Wires. WBP/gameplay stay available; duplicated visuals remain in the safe uncut state. Bomb=%s"),
						WireIndex,
						*GetNameSafe(*ExistingWireComponent),
						*GetNameSafe(WireComponent),
						*GetNameSafe(this));
				}
				else
				{
					UE_LOG(LogTemp, Error,
						TEXT("[BombDefusal] Invalid WireComponent mapping. Bomb=%s Component=%s WireIndex=%d HasDAEntry=%s. WBP/gameplay stay available; this world visual remains uncut until the mapping is fixed."),
						*GetNameSafe(this),
						*GetNameSafe(WireComponent),
						WireIndex,
						bHasDefinition ? TEXT("true") : TEXT("false"));
				}
			}
			continue;
		}

		FirstWireComponentByIndex.Add(WireIndex, WireComponent);
		ComponentWireIndices.Add(WireIndex);
	}

	TSet<FName> ComponentButtonIds;
	TMap<FName, const UREBombButtonComponent*> FirstButtonComponentById;
	for (const UREBombButtonComponent* ButtonComponent : IntegratedButtonComponents)
	{
		if (IsValid(ButtonComponent) == false)
		{
			continue;
		}

		const FName ButtonId = ButtonComponent->GetButtonId();
		FREBombButtonDefinition ButtonDefinition;
		const bool bHasDefinition = ButtonId.IsNone() == false
			&& PatternData->GetButtonDefinition(ButtonId, ButtonDefinition);
		const UREBombButtonComponent* const* ExistingButtonComponent = FirstButtonComponentById.Find(ButtonId);
		const bool bDuplicate = ExistingButtonComponent != nullptr;
		const bool bMappingValid = ButtonId.IsNone() == false
			&& bHasDefinition == true
			&& bDuplicate == false;

		if (bMappingValid == false)
		{
			bValid = false;
			if (bLogWarnings == true)
			{
				if (bDuplicate == true)
				{
					UE_LOG(LogTemp, Error,
						TEXT("[BombDefusal] Duplicate ButtonId='%s'. Components '%s' and '%s' point to the same logical button. Give every physical button a unique ID matching DA.Buttons. WBP/gameplay stay available; duplicated visuals remain in the safe released state. Bomb=%s"),
						*ButtonId.ToString(),
						*GetNameSafe(*ExistingButtonComponent),
						*GetNameSafe(ButtonComponent),
						*GetNameSafe(this));
				}
				else
				{
					UE_LOG(LogTemp, Error,
						TEXT("[BombDefusal] Invalid ButtonComponent mapping. Bomb=%s Component=%s ButtonId=%s HasDAEntry=%s. WBP/gameplay stay available; this world visual remains released until the mapping is fixed."),
						*GetNameSafe(this),
						*GetNameSafe(ButtonComponent),
						*ButtonId.ToString(),
						bHasDefinition ? TEXT("true") : TEXT("false"));
				}
			}
			continue;
		}

		FirstButtonComponentById.Add(ButtonId, ButtonComponent);
		ComponentButtonIds.Add(ButtonId);
	}

	for (const FREBombWireDefinition& WireDefinition : PatternData->Wires)
	{
		if (ComponentWireIndices.Contains(WireDefinition.WireIndex) == false)
		{
			bValid = false;
			if (bLogWarnings == true)
			{
				UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] DA wire has no visual WireComponent in the integrated Blueprint. WBP/gameplay stay available; only the world visual is missing. Bomb=%s WireIndex=%d"), *GetNameSafe(this), WireDefinition.WireIndex);
			}
		}
	}

	TArray<FREBombButtonDefinition> ResolvedButtons;
	PatternData->GetResolvedButtonDefinitions(ResolvedButtons);
	for (const FREBombButtonDefinition& ButtonDefinition : ResolvedButtons)
	{
		if (ComponentButtonIds.Contains(ButtonDefinition.ButtonId) == false)
		{
			bValid = false;
			if (bLogWarnings == true)
			{
				UE_LOG(LogTemp, Warning, TEXT("[BombDefusal] DA button has no visual ButtonComponent in the integrated Blueprint. WBP/gameplay stay available; only the world visual is missing. Bomb=%s ButtonId=%s"), *GetNameSafe(this), *ButtonDefinition.ButtonId.ToString());
			}
		}
	}

	return bValid;
}

bool AREBombDefusalManager::ValidateCurrentWireStep(int32 WireIndex, FText& OutFailureMessage) const
{
	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false || CurrentStep.StepType != EREBombStepType::CutWire)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	if (IsWireCut(WireIndex) == true || WireIndex != CurrentStep.WireIndex)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	return true;
}

bool AREBombDefusalManager::ValidateCurrentButtonStep(FName ButtonId, bool bNextPressed, FText& OutFailureMessage) const
{
	FREBombStep CurrentStep;
	if (GetCurrentStep(CurrentStep) == false || CurrentStep.StepType != EREBombStepType::ButtonState)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	if (ButtonId != CurrentStep.ButtonId || bNextPressed != CurrentStep.bRequiredButtonPressed)
	{
		OutFailureMessage = BuildFailureFeedbackMessage();
		return false;
	}

	return true;
}

APlayerState* AREBombDefusalManager::ResolvePlayerState(AActor* Actor) const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		return PlayerController->PlayerState;
	}

	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Pawn->GetPlayerState();
	}

	return nullptr;
}

void AREBombDefusalManager::RefreshNativeDisplay()
{
	const int32 TotalSeconds = FMath::CeilToInt(RemainingTimeSeconds);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	if (IsValid(TimerText) == true)
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}

	if (IsValid(StepText) == true)
	{
		const int32 TotalSteps = GetTotalStepCount();
		const int32 DisplayStep = TotalSteps > 0 ? FMath::Min(CurrentStepIndex + 1, TotalSteps) : 0;
		StepText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), DisplayStep, TotalSteps)));
	}
}

void AREBombDefusalManager::SynchronizePromptCandidatesFromCurrentOverlaps()
{
	if (IsValid(InteractionCollision) == false)
	{
		LocalPromptCandidates.Reset();
		return;
	}

	TArray<AActor*> OverlappingActors;
	InteractionCollision->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	LocalPromptCandidates.Reset();
	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (ShouldTrackPromptActor(OverlappingActor) == true)
		{
			LocalPromptCandidates.AddUnique(OverlappingActor);
		}
	}
}

void AREBombDefusalManager::RefreshInteractionPromptVisibility()
{
	bool bHasValidCandidate = false;
	for (int32 Index = LocalPromptCandidates.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = LocalPromptCandidates[Index].Get();
		if (ShouldTrackPromptActor(Candidate) == false)
		{
			LocalPromptCandidates.RemoveAtSwap(Index);
			continue;
		}

		if (ShouldShowPromptForActor(Candidate) == true)
		{
			bHasValidCandidate = true;
		}
	}

	const bool bWidgetOpen = IsValid(ActiveBombDefusalWidget) == true && ActiveBombDefusalWidget->IsInViewport() == true;
	SetInteractionPromptVisible(bHasValidCandidate == true && bWidgetOpen == false);
}

void AREBombDefusalManager::SetInteractionPromptVisible(bool bVisible)
{
	const bool bUseWidgetPrompt = IsValid(InteractionPromptWidgetComponent) == true
		&& IsValid(InteractionPromptWidgetClass) == true;
	const bool bUseTextFallback = bUseWidgetPrompt == false
		&& bUseNativePromptFallback == true
		&& IsValid(InteractionPromptTextRender) == true;

	if (bUseWidgetPrompt == true)
	{
		InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);
		InteractionPromptWidgetComponent->SetVisibility(bVisible);
		InteractionPromptWidgetComponent->SetHiddenInGame(!bVisible);
	}
	else if (IsValid(InteractionPromptWidgetComponent) == true)
	{
		InteractionPromptWidgetComponent->SetVisibility(false);
		InteractionPromptWidgetComponent->SetHiddenInGame(true);
	}

	if (bUseTextFallback == true)
	{
		InteractionPromptTextRender->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptTextRender->SetWorldSize(InteractionPromptWorldSize);
		InteractionPromptTextRender->SetVisibility(bVisible);
		InteractionPromptTextRender->SetHiddenInGame(!bVisible);
	}
	else if (IsValid(InteractionPromptTextRender) == true)
	{
		InteractionPromptTextRender->SetVisibility(false);
		InteractionPromptTextRender->SetHiddenInGame(true);
	}

	if (bVisible == true)
	{
		RefreshInteractionPromptText();
	}
}

void AREBombDefusalManager::RefreshInteractionPromptText() const
{
	if (IsValid(InteractionPromptTextRender) == true)
	{
		InteractionPromptTextRender->SetText(InteractionPromptText);
	}

	if (IsValid(InteractionPromptWidgetComponent) == false || IsValid(InteractionPromptWidgetClass) == false)
	{
		return;
	}

	if (IsValid(InteractionPromptWidgetComponent->GetUserWidgetObject()) == false)
	{
		InteractionPromptWidgetComponent->InitWidget();
	}

	UUserWidget* PromptWidget = InteractionPromptWidgetComponent->GetUserWidgetObject();
	if (IsValid(PromptWidget) == false)
	{
		return;
	}

	static const FName SupportedPromptTextNames[] =
	{
		TEXT("TXT_InteractPrompt"),
		TEXT("TextBlock_InputAction"),
		TEXT("TXT_Prompt")
	};

	for (const FName& WidgetName : SupportedPromptTextNames)
	{
		if (UTextBlock* PromptTextBlock = Cast<UTextBlock>(PromptWidget->GetWidgetFromName(WidgetName)))
		{
			PromptTextBlock->SetText(InteractionPromptText);
			break;
		}
	}
}

bool AREBombDefusalManager::ShouldTrackPromptActor(AActor* Actor) const
{
	return IsValid(Cast<APawn>(Actor)) == true;
}

bool AREBombDefusalManager::ShouldShowPromptForActor(AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	const bool bHasPromptPresenter = IsValid(InteractionPromptWidgetClass) == true || bUseNativePromptFallback == true;
	return IsValid(Pawn) == true
		&& Pawn->IsLocallyControlled() == true
		&& bHasPromptPresenter == true
		&& CanInteractWithBomb(Actor) == true;
}
