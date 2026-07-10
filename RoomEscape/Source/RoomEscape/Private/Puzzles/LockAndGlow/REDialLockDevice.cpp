#include "Puzzles/LockAndGlow/REDialLockDevice.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/LockAndGlow/RELockAndGlowClueManager.h"
#include "UI/REDialLockWidget.h"

AREDialLockDevice::AREDialLockDevice()
{
	bReplicates = true;

	LockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockMesh"));
	LockMesh->SetupAttachment(SceneRoot);
	LockMesh->SetCollisionProfileName(TEXT("BlockAll"));

	InteractionCollision->SetBoxExtent(FVector(100.0, 100.0, 90.0));
	InteractionPromptText = FText::FromString(TEXT("자물쇠 조작"));
	InteractionPromptRelativeLocation = FVector(0.0, 0.0, 110.0);
}

void AREDialLockDevice::BeginPlay()
{
	Super::BeginPlay();

	NormalizeDigits();
	if (ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager())
	{
		LockManager->RegisterLockDevice(this);
	}
}

void AREDialLockDevice::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentDigits);
	DOREPLIFETIME(ThisClass, RuntimeCodeLength);
	DOREPLIFETIME(ThisClass, bUnlocked);
}

void AREDialLockDevice::SetLockAndGlowManager(ARELockAndGlowClueManager* InManager)
{
	SetPuzzleManager(InManager);
}

ARELockAndGlowClueManager* AREDialLockDevice::GetLockAndGlowManager() const
{
	return Cast<ARELockAndGlowClueManager>(PuzzleManager);
}

void AREDialLockDevice::GetCurrentDigits(TArray<int32>& OutDigits) const
{
	OutDigits = CurrentDigits;
}

int32 AREDialLockDevice::GetCodeLength() const
{
	return FMath::Clamp(RuntimeCodeLength, 1, 8);
}

bool AREDialLockDevice::IsUnlocked() const
{
	return bUnlocked;
}

void AREDialLockDevice::ApplyServerCodeLength(int32 NewCodeLength)
{
	if (HasAuthority() == false)
	{
		return;
	}

	RuntimeCodeLength = FMath::Clamp(NewCodeLength, 1, 8);
	NormalizeDigits();
	OnRep_CodeLength();
	OnRep_Digits();
}

void AREDialLockDevice::ApplyServerUnlockedState(bool bNewUnlocked)
{
	if (HasAuthority() == false || bUnlocked == bNewUnlocked)
	{
		return;
	}

	bUnlocked = bNewUnlocked;
	OnRep_Unlocked();
}

void AREDialLockDevice::ServerIncrementDigit_Implementation(int32 DigitIndex)
{
	if (CanUseElement(GetOwner()) == false)
	{
		return;
	}

	const int32 CurrentValue = CurrentDigits.IsValidIndex(DigitIndex) == true ? CurrentDigits[DigitIndex] : 0;
	SetDigitInternal(DigitIndex, CurrentValue + 1);
}

void AREDialLockDevice::ServerDecrementDigit_Implementation(int32 DigitIndex)
{
	if (CanUseElement(GetOwner()) == false)
	{
		return;
	}

	const int32 CurrentValue = CurrentDigits.IsValidIndex(DigitIndex) == true ? CurrentDigits[DigitIndex] : 0;
	SetDigitInternal(DigitIndex, CurrentValue - 1);
}

void AREDialLockDevice::ServerSetDigit_Implementation(int32 DigitIndex, int32 DigitValue)
{
	if (CanUseElement(GetOwner()) == false)
	{
		return;
	}

	SetDigitInternal(DigitIndex, DigitValue);
}

void AREDialLockDevice::ServerSubmitCode_Implementation(AActor* Interactor)
{
	ARELockAndGlowClueManager* LockManager = GetLockAndGlowManager();
	if (IsValid(LockManager) == false || CanUseElement(Interactor) == false)
	{
		return;
	}

	LockManager->SubmitCode(this, Interactor, CurrentDigits);
}

void AREDialLockDevice::CloseDialLockWidget()
{
	if (IsValid(ActiveDialLockWidget) == true)
	{
		ActiveDialLockWidget->RemoveFromParent();
		ActiveDialLockWidget = nullptr;
	}
}

bool AREDialLockDevice::CanUseElement(AActor* Interactor) const
{
	return bUnlocked == false && Super::CanUseElement(Interactor) == true;
}

void AREDialLockDevice::HandleInteract(AActor* Interactor)
{
	APlayerController* PlayerController = ResolvePlayerController(Interactor);
	if (IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Open rejected. Could not resolve PlayerController. Device=%s Interactor=%s"), *GetNameSafe(this), *GetNameSafe(Interactor));
		return;
	}

	if (IsValid(DialLockWidgetClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Open rejected. DialLockWidgetClass is not assigned. Device=%s"), *GetNameSafe(this));
		return;
	}

	SetOwner(PlayerController);
	ForceNetUpdate();

	if (PlayerController->IsLocalController() == true)
	{
		OpenDialLockWidgetLocal(PlayerController, DialLockWidgetClass, CurrentDigits);
		return;
	}

	ClientOpenDialLockWidget(DialLockWidgetClass, CurrentDigits);
}

void AREDialLockDevice::OnRep_Digits()
{
	NormalizeDigits();
	OnDialLockDigitsChanged.Broadcast(CurrentDigits);
	ReceiveDigitsChanged(CurrentDigits);
}

void AREDialLockDevice::OnRep_CodeLength()
{
	NormalizeDigits();
	OnDialLockDigitsChanged.Broadcast(CurrentDigits);
	ReceiveDigitsChanged(CurrentDigits);
}

void AREDialLockDevice::OnRep_Unlocked()
{
	/*
	 * Do not close the active widget here. The widget listens for the submit result
	 * and owns the success/failure feedback timing, so players can actually see
	 * whether the submitted code was correct before the UI closes.
	 */
	OnDialLockUnlockedChanged.Broadcast(bUnlocked);
	ReceiveUnlockedChanged(bUnlocked);
}

void AREDialLockDevice::ClientOpenDialLockWidget_Implementation(TSubclassOf<UREDialLockWidget> InWidgetClass, const TArray<int32>& InitialDigits)
{
	APlayerController* LocalPlayerController = Cast<APlayerController>(GetOwner());
	if (IsValid(LocalPlayerController) == false || LocalPlayerController->IsLocalController() == false)
	{
		LocalPlayerController = GetWorld() != nullptr ? GetWorld()->GetFirstPlayerController() : nullptr;
	}

	OpenDialLockWidgetLocal(LocalPlayerController, InWidgetClass, InitialDigits);
}

APlayerController* AREDialLockDevice::ResolvePlayerController(AActor* Actor) const
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

void AREDialLockDevice::OpenDialLockWidgetLocal(APlayerController* PlayerController, TSubclassOf<UREDialLockWidget> InWidgetClass, const TArray<int32>& InitialDigits)
{
	if (IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Local open rejected. PlayerController is invalid. Device=%s"), *GetNameSafe(this));
		return;
	}

	if (PlayerController->IsLocalController() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Local open rejected. PlayerController is not local. Device=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(PlayerController));
		return;
	}

	if (IsValid(InWidgetClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Local open rejected. Widget class is invalid. Device=%s"), *GetNameSafe(this));
		return;
	}

	if (IsValid(ActiveDialLockWidget) == false || ActiveDialLockWidget->GetClass() != InWidgetClass)
	{
		ActiveDialLockWidget = CreateWidget<UREDialLockWidget>(PlayerController, InWidgetClass);
	}

	if (IsValid(ActiveDialLockWidget) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DialLock] Failed to create widget. Device=%s WidgetClass=%s"), *GetNameSafe(this), *GetNameSafe(InWidgetClass));
		return;
	}

	if (ActiveDialLockWidget->IsInViewport() == false)
	{
		ActiveDialLockWidget->AddToViewport(100);
	}

	ActiveDialLockWidget->InitializeDialLock(this, InitialDigits);
}

void AREDialLockDevice::NormalizeDigits()
{
	const int32 DesiredLength = GetCodeLength();
	if (CurrentDigits.Num() > DesiredLength)
	{
		CurrentDigits.SetNum(DesiredLength);
	}

	while (CurrentDigits.Num() < DesiredLength)
	{
		CurrentDigits.Add(0);
	}

	for (int32 DigitIndex = 0; DigitIndex < CurrentDigits.Num(); ++DigitIndex)
	{
		CurrentDigits[DigitIndex] = FMath::Clamp(CurrentDigits[DigitIndex], 0, 9);
	}
}

void AREDialLockDevice::SetDigitInternal(int32 DigitIndex, int32 DigitValue)
{
	NormalizeDigits();
	if (CurrentDigits.IsValidIndex(DigitIndex) == false)
	{
		return;
	}

	const int32 WrappedDigit = (DigitValue % 10 + 10) % 10;
	if (CurrentDigits[DigitIndex] == WrappedDigit)
	{
		return;
	}

	CurrentDigits[DigitIndex] = WrappedDigit;
	OnRep_Digits();
}
