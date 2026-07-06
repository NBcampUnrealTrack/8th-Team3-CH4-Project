#include "Puzzles/BombDefusal/REDefusalManual.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "Puzzles/BombDefusal/REBombPatternData.h"
#include "UI/REBombManualWidget.h"

AREDefusalManual::AREDefusalManual()
{
	ManualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ManualMesh"));
	ManualMesh->SetupAttachment(SceneRoot);
	ManualMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(70.0, 70.0, 70.0));
}

void AREDefusalManual::SetBombManager(AREBombDefusalManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREBombDefusalManager* AREDefusalManual::GetBombManager() const
{
	return Cast<AREBombDefusalManager>(PuzzleManager);
}

UREBombPatternData* AREDefusalManual::GetManualPatternData() const
{
	if (IsValid(OverridePatternData) == true)
	{
		return OverridePatternData;
	}

	const AREBombDefusalManager* BombManager = GetBombManager();
	return IsValid(BombManager) == true ? BombManager->GetPatternData() : nullptr;
}

void AREDefusalManual::CloseManualWidget()
{
	if (IsValid(ActiveManualWidget) == true)
	{
		ActiveManualWidget->RemoveFromParent();
		ActiveManualWidget = nullptr;
	}
}

bool AREDefusalManual::CanUseElement(AActor* Interactor) const
{
	return IsValid(Interactor) == true && IsValid(ManualWidgetClass) == true && IsValid(GetManualPatternData()) == true && Super::CanUseElement(Interactor) == true;
}

void AREDefusalManual::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	UREBombPatternData* ManualPatternData = GetManualPatternData();
	if (IsValid(PlayerController) == false || IsValid(ManualWidgetClass) == false || IsValid(ManualPatternData) == false)
	{
		return;
	}

	SetOwner(PlayerController);
	const FText ManualText = ManualPatternData->BuildManualText();
	if (PlayerController->IsLocalController() == true)
	{
		OpenManualWidgetLocal(PlayerController, ManualWidgetClass, ManualTitle, ManualText);
		return;
	}

	ClientOpenManualWidget(PlayerController, ManualWidgetClass, ManualTitle, ManualText);
}

void AREDefusalManual::ClientOpenManualWidget_Implementation(APlayerController* OwningPlayerController, TSubclassOf<UREBombManualWidget> InWidgetClass, const FText& InManualTitle, const FText& InManualText)
{
	OpenManualWidgetLocal(OwningPlayerController, InWidgetClass, InManualTitle, InManualText);
}

APlayerController* AREDefusalManual::ResolvePlayerController(AActor* Actor) const
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

void AREDefusalManual::OpenManualWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREBombManualWidget> InWidgetClass, const FText& InManualTitle, const FText& InManualText)
{
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false || IsValid(InWidgetClass) == false)
	{
		return;
	}

	if (IsValid(ActiveManualWidget) == false || ActiveManualWidget->GetClass() != InWidgetClass)
	{
		ActiveManualWidget = CreateWidget<UREBombManualWidget>(PlayerController, InWidgetClass);
	}

	if (IsValid(ActiveManualWidget) == false)
	{
		return;
	}

	if (ActiveManualWidget->IsInViewport() == false)
	{
		ActiveManualWidget->AddToViewport(100);
	}

	ActiveManualWidget->InitializeManual(this, InManualTitle, InManualText);
}
