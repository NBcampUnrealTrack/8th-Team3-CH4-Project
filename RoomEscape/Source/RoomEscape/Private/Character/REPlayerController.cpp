#include "Character/REPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "Game/RENotifySubsystem.h"

void AREPlayerController::RETestNotify(const FString& Message)
{
	if (URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this))
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Debug::Test, Message);
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
