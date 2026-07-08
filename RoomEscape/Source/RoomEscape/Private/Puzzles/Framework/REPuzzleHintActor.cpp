#include "Puzzles/Framework/REPuzzleHintActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/REPuzzleHintWidget.h"

AREPuzzleHintActor::AREPuzzleHintActor()
{
	bAlwaysRelevant = true;

	HintMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HintMesh"));
	HintMesh->SetupAttachment(SceneRoot);
	HintMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(80.0, 80.0, 120.0));
}

void AREPuzzleHintActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bUnlocked);
}

void AREPuzzleHintActor::SetUnlocked(bool bNewUnlocked)
{
	if (HasAuthority() == false || bUnlocked == bNewUnlocked)
	{
		return;
	}

	bUnlocked = bNewUnlocked;
	OnRep_Unlocked();
}

bool AREPuzzleHintActor::IsUnlocked() const
{
	return bUnlocked;
}

FText AREPuzzleHintActor::GetHintTitle() const
{
	return HintTitle;
}

FText AREPuzzleHintActor::GetHintText() const
{
	return HintText;
}

void AREPuzzleHintActor::CloseHintWidget()
{
	if (IsValid(ActiveHintWidget) == true)
	{
		ActiveHintWidget->RemoveFromParent();
		ActiveHintWidget = nullptr;
	}
}

bool AREPuzzleHintActor::CanUseElement(AActor* Interactor) const
{
	return IsValid(Interactor) == true;
}

void AREPuzzleHintActor::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Interact received. Actor=%s Interactor=%s Controller=%s WidgetClass=%s Unlocked=%d"), *GetNameSafe(this), *GetNameSafe(Interactor), *GetNameSafe(PlayerController), *GetNameSafe(HintWidgetClass.Get()), bUnlocked ? 1 : 0);

	if (bUnlocked == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Open blocked because hint is locked. Actor=%s"), *GetNameSafe(this));
		return;
	}

	if (IsValid(HintWidgetClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Open blocked because HintWidgetClass is empty. Actor=%s"), *GetNameSafe(this));
		return;
	}

	if (IsValid(PlayerController) == false)
	{
		PlayerController = ResolveLocalPlayerController(nullptr);
	}

	if (IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Open blocked because PlayerController could not be resolved. Actor=%s Interactor=%s"), *GetNameSafe(this), *GetNameSafe(Interactor));
		return;
	}

	SetOwner(PlayerController);

	if (PlayerController->IsLocalController() == true)
	{
		OpenHintWidgetLocal(PlayerController, HintWidgetClass, HintTitle, HintText);
		return;
	}

	ClientOpenHintWidget(PlayerController, HintWidgetClass, HintTitle, HintText);
}

void AREPuzzleHintActor::OnRep_Unlocked()
{
	ReceiveUnlockedChanged(bUnlocked);
}

void AREPuzzleHintActor::ClientOpenHintWidget_Implementation(APlayerController* OwningPlayerController, TSubclassOf<UREPuzzleHintWidget> InWidgetClass, const FText& InHintTitle, const FText& InHintText)
{
	APlayerController* LocalPlayerController = ResolveLocalPlayerController(OwningPlayerController);
	OpenHintWidgetLocal(LocalPlayerController, InWidgetClass, InHintTitle, InHintText);
}

APlayerController* AREPuzzleHintActor::ResolvePlayerController(AActor* Actor) const
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

APlayerController* AREPuzzleHintActor::ResolveLocalPlayerController(APlayerController* PreferredPlayerController) const
{
	if (IsValid(PreferredPlayerController) == true && PreferredPlayerController->IsLocalController() == true)
	{
		return PreferredPlayerController;
	}

	if (APlayerController* OwnerPlayerController = Cast<APlayerController>(GetOwner()))
	{
		if (OwnerPlayerController->IsLocalController() == true)
		{
			return OwnerPlayerController;
		}
	}

	return UGameplayStatics::GetPlayerController(this, 0);
}

void AREPuzzleHintActor::OpenHintWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREPuzzleHintWidget> InWidgetClass, const FText& InHintTitle, const FText& InHintText)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(InWidgetClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Local open failed. Actor=%s LocalController=%s WidgetClass=%s"), *GetName(), *GetNameSafe(PlayerController), *GetNameSafe(InWidgetClass.Get()));
		return;
	}

	if (IsValid(ActiveHintWidget) == false || ActiveHintWidget->GetClass() != InWidgetClass)
	{
		ActiveHintWidget = CreateWidget<UREPuzzleHintWidget>(PlayerController, InWidgetClass);
	}

	if (IsValid(ActiveHintWidget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Widget creation failed. Actor=%s Controller=%s WidgetClass=%s"), *GetName(), *GetNameSafe(PlayerController), *GetNameSafe(InWidgetClass.Get()));
		return;
	}

	if (ActiveHintWidget->IsInViewport() == false)
	{
		ActiveHintWidget->AddToViewport(100);
	}

	ActiveHintWidget->InitializeHint(this, InHintTitle, InHintText);

	UE_LOG(LogTemp, Warning, TEXT("[PuzzleHint] Widget opened. Actor=%s Controller=%s Widget=%s"), *GetName(), *GetNameSafe(PlayerController), *GetNameSafe(ActiveHintWidget));
}
