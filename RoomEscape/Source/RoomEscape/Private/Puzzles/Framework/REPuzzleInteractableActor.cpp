#include "Puzzles/Framework/REPuzzleInteractableActor.h"

#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/TextBlock.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Puzzles/Framework/REPuzzleManager.h"

AREPuzzleInteractableActor::AREPuzzleInteractableActor()
{
	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(SceneRoot);
	InteractionCollision->SetBoxExtent(FVector(100.0, 100.0, 100.0));
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidgetComponent->SetupAttachment(SceneRoot);
	InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptWidgetComponent->SetVisibility(false);
	InteractionPromptWidgetComponent->SetHiddenInGame(true);
	InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
	InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);

	InteractionPromptTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InteractionPromptTextRender"));
	InteractionPromptTextRender->SetupAttachment(SceneRoot);
	InteractionPromptTextRender->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptTextRender->SetHiddenInGame(true);
	InteractionPromptTextRender->SetVisibility(false);
	InteractionPromptTextRender->SetHorizontalAlignment(EHTA_Center);
	InteractionPromptTextRender->SetVerticalAlignment(EVRTA_TextCenter);
	InteractionPromptTextRender->SetTextRenderColor(FColor::White);
	InteractionPromptTextRender->SetWorldSize(NativePromptWorldSize);
	InteractionPromptTextRender->SetRelativeLocation(InteractionPromptRelativeLocation);

	InteractionPromptText = FText::FromString(TEXT("상호작용"));
}

void AREPuzzleInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(InteractionPromptWidgetComponent) == true)
	{
		InteractionPromptWidgetComponent->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptWidgetComponent->SetDrawSize(InteractionPromptDrawSize);
		if (IsValid(InteractionPromptWidgetClass) == true)
		{
			InteractionPromptWidgetComponent->SetWidgetClass(InteractionPromptWidgetClass);
		}
	}

	if (IsValid(InteractionPromptTextRender) == true)
	{
		InteractionPromptTextRender->SetRelativeLocation(InteractionPromptRelativeLocation);
		InteractionPromptTextRender->SetWorldSize(NativePromptWorldSize);
	}

	SetInteractionPromptVisible(false);
	RefreshInteractionPromptText();
	BindPuzzleManagerStateChanged();

	if (IsValid(InteractionCollision) == true)
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(this, &AREPuzzleInteractableActor::OnInteractionPromptBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(this, &AREPuzzleInteractableActor::OnInteractionPromptEndOverlap);
	}
}

void AREPuzzleInteractableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPuzzleManagerStateChanged();
	PromptCandidateActors.Reset();
	Super::EndPlay(EndPlayReason);
}

bool AREPuzzleInteractableActor::CanInteract(AActor* Interactor) const
{
	return CanUseElement(Interactor);
}

void AREPuzzleInteractableActor::Interact_Implementation(AActor* Interactor)
{
	ProcessServerInteract(Interactor);
}

void AREPuzzleInteractableActor::ProcessServerInteract(AActor* Interactor)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleInteractable] Rejected because actor has no authority. Target=%s Interactor=%s"), *GetNameSafe(this), *GetNameSafe(Interactor));
		return;
	}

	if (CanUseElement(Interactor) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleInteractable] Rejected by CanUseElement. Target=%s Interactor=%s Manager=%s StateActive=%d"), *GetNameSafe(this), *GetNameSafe(Interactor), *GetNameSafe(PuzzleManager.Get()), IsValid(PuzzleManager) && PuzzleManager->IsActive() ? 1 : 0);
		return;
	}

	HandleInteract(Interactor);
}

bool AREPuzzleInteractableActor::CanUseElement(AActor* Interactor) const
{
	return IsValid(PuzzleManager) == true && IsValid(Interactor) == true && PuzzleManager->IsActive() == true;
}

void AREPuzzleInteractableActor::HandleInteract(AActor* Interactor)
{
}

void AREPuzzleInteractableActor::HandlePuzzleManagerChanged()
{
	Super::HandlePuzzleManagerChanged();
	BindPuzzleManagerStateChanged();
	RefreshInteractionPromptVisibility();
}

void AREPuzzleInteractableActor::OnInteractionPromptBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldTrackPromptActor(OtherActor) == true)
	{
		AddPromptCandidate(OtherActor);
		RefreshInteractionPromptVisibility();
	}
}

void AREPuzzleInteractableActor::OnInteractionPromptEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ShouldTrackPromptActor(OtherActor) == true)
	{
		RemovePromptCandidate(OtherActor);
		RefreshInteractionPromptVisibility();
	}
}

void AREPuzzleInteractableActor::OnBoundPuzzleStateChanged(EREPuzzleState NewState)
{
	RefreshInteractionPromptVisibility();
}

bool AREPuzzleInteractableActor::ShouldTrackPromptActor(AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return IsValid(Pawn) == true && Pawn->IsLocallyControlled() == true;
}

bool AREPuzzleInteractableActor::ShouldShowPromptForActor(AActor* Actor) const
{
	const bool bHasPromptPresenter = IsValid(InteractionPromptWidgetClass) == true || bUseNativePromptFallback == true;
	return ShouldTrackPromptActor(Actor) == true && bHasPromptPresenter == true && CanUseElement(Actor) == true;
}

void AREPuzzleInteractableActor::AddPromptCandidate(AActor* Actor)
{
	if (IsValid(Actor) == false)
	{
		return;
	}

	for (const TWeakObjectPtr<AActor>& Candidate : PromptCandidateActors)
	{
		if (Candidate.Get() == Actor)
		{
			return;
		}
	}

	PromptCandidateActors.Add(Actor);
}

void AREPuzzleInteractableActor::RemovePromptCandidate(AActor* Actor)
{
	PromptCandidateActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& Candidate)
	{
		return Candidate.IsValid() == false || Candidate.Get() == Actor;
	});
}

void AREPuzzleInteractableActor::RefreshInteractionPromptVisibility()
{
	bool bShouldShow = false;

	for (int32 Index = PromptCandidateActors.Num() - 1; Index >= 0; --Index)
	{
		AActor* CandidateActor = PromptCandidateActors[Index].Get();
		if (IsValid(CandidateActor) == false)
		{
			PromptCandidateActors.RemoveAtSwap(Index);
			continue;
		}

		if (ShouldShowPromptForActor(CandidateActor) == true)
		{
			bShouldShow = true;
		}
	}

	SetInteractionPromptVisible(bShouldShow);
}

void AREPuzzleInteractableActor::SetInteractionPromptVisible(bool bVisible)
{
	const bool bUseWidgetPrompt = IsValid(InteractionPromptWidgetComponent) == true && IsValid(InteractionPromptWidgetClass) == true;
	const bool bUseTextFallback = bUseWidgetPrompt == false && bUseNativePromptFallback == true && IsValid(InteractionPromptTextRender) == true;

	if (bUseWidgetPrompt == true)
	{
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

void AREPuzzleInteractableActor::RefreshInteractionPromptText() const
{
	if (IsValid(InteractionPromptTextRender) == true)
	{
		InteractionPromptTextRender->SetText(InteractionPromptText);
	}

	if (IsValid(InteractionPromptWidgetComponent) == false)
	{
		return;
	}

	if (IsValid(InteractionPromptWidgetComponent->GetUserWidgetObject()) == false && IsValid(InteractionPromptWidgetClass) == true)
	{
		InteractionPromptWidgetComponent->InitWidget();
	}

	UUserWidget* PromptWidget = InteractionPromptWidgetComponent->GetUserWidgetObject();
	if (IsValid(PromptWidget) == false)
	{
		return;
	}

	if (UTextBlock* PromptTextBlock = Cast<UTextBlock>(PromptWidget->GetWidgetFromName(TEXT("TXT_InteractPrompt"))))
	{
		PromptTextBlock->SetText(InteractionPromptText);
	}
}

void AREPuzzleInteractableActor::BindPuzzleManagerStateChanged()
{
	AREPuzzleManager* CurrentManager = PuzzleManager.Get();
	if (BoundPuzzleManager.Get() == CurrentManager)
	{
		return;
	}

	UnbindPuzzleManagerStateChanged();

	if (IsValid(CurrentManager) == true)
	{
		CurrentManager->OnPuzzleStateChanged.AddUniqueDynamic(this, &AREPuzzleInteractableActor::OnBoundPuzzleStateChanged);
		BoundPuzzleManager = CurrentManager;
	}
}

void AREPuzzleInteractableActor::UnbindPuzzleManagerStateChanged()
{
	if (AREPuzzleManager* Manager = BoundPuzzleManager.Get())
	{
		Manager->OnPuzzleStateChanged.RemoveDynamic(this, &AREPuzzleInteractableActor::OnBoundPuzzleStateChanged);
	}

	BoundPuzzleManager.Reset();
}
