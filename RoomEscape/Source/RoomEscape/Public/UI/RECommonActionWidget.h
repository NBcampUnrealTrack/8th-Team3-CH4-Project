// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "RECommonActionWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URECommonActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:
	virtual FSlateBrush GetIcon() const override;

protected:
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
