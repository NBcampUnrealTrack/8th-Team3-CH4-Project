#include "Character/REPlayerController.h"

#include "EnhancedInputSubsystems.h"

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
