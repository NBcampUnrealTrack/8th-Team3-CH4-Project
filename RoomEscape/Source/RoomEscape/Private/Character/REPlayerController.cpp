#include "Character/REPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "Game/RENotifySubsystem.h"
#include "GameFramework/Pawn.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"

void AREPlayerController::RETestNotify(const FString& Message)
{
	if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Debug::Test, Message);
	}
}

void AREPlayerController::ServerSubmitBombWireSelection_Implementation(AREBombDefusalManager* BombManager, int32 WireIndex)
{
	APawn* ControlledPawn = GetPawn();
	AActor* Interactor = IsValid(ControlledPawn) == true ? static_cast<AActor*>(ControlledPawn) : static_cast<AActor*>(this);
	if (IsValid(BombManager) == true)
	{
		BombManager->SubmitWireSelectionFromWidget(WireIndex, Interactor);
	}
}

void AREPlayerController::ServerSubmitBombButtonSelection_Implementation(AREBombDefusalManager* BombManager, FName ButtonId)
{
	APawn* ControlledPawn = GetPawn();
	AActor* Interactor = IsValid(ControlledPawn) == true ? static_cast<AActor*>(ControlledPawn) : static_cast<AActor*>(this);
	if (IsValid(BombManager) == true)
	{
		BombManager->SubmitButtonSelectionFromWidget(ButtonId, Interactor);
	}
}

void AREPlayerController::ClientOpenBombDefusalWidget_Implementation(AREBombDefusalManager* BombManager)
{
	if (IsLocalController() == true && IsValid(BombManager) == true)
	{
		BombManager->OpenBombDefusalWidgetForLocalPlayer(this);
	}
}

void AREPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (Subsystem)
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, MappingPriority);
			}
		}
	}
}
