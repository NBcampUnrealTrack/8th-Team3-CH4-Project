#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/TextBlock.h"
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
		SetInteractionPromptVisible(false);
		RefreshInteractionPromptText();
	}

	if (IsValid(InteractionCollision) == true)
	{
		InteractionCollision->OnComponentBeginOverlap.AddDynamic(this, &AREPuzzleInteractableActor::OnInteractionPromptBeginOverlap);
		InteractionCollision->OnComponentEndOverlap.AddDynamic(this, &AREPuzzleInteractableActor::OnInteractionPromptEndOverlap);
	}
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

void AREPuzzleInteractableActor::OnInteractionPromptBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldShowPromptForActor(OtherActor) == true)
	{
		SetInteractionPromptVisible(true);
	}
}

void AREPuzzleInteractableActor::OnInteractionPromptEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ShouldShowPromptForActor(OtherActor) == true)
	{
		SetInteractionPromptVisible(false);
	}
}

bool AREPuzzleInteractableActor::ShouldShowPromptForActor(AActor* Actor) const
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return IsValid(Pawn) == true && Pawn->IsLocallyControlled() == true && IsValid(InteractionPromptWidgetClass) == true;
}

void AREPuzzleInteractableActor::SetInteractionPromptVisible(bool bVisible)
{
	if (IsValid(InteractionPromptWidgetComponent) == false || IsValid(InteractionPromptWidgetClass) == false)
	{
		return;
	}

	InteractionPromptWidgetComponent->SetVisibility(bVisible);
	InteractionPromptWidgetComponent->SetHiddenInGame(!bVisible);
	if (bVisible == true)
	{
		RefreshInteractionPromptText();
	}
}

void AREPuzzleInteractableActor::RefreshInteractionPromptText() const
{
	if (IsValid(InteractionPromptWidgetComponent) == false)
	{
		return;
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
