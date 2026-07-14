// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "REWidgetManager.generated.h"

/**
 * Not Completed
 * Test WidgetManager Using GameplayTags
 */
UCLASS()
class ROOMESCAPE_API UREWidgetManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
};
