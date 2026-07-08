// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SingleQuickSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API USingleQuickSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
protected:
	TObjectPtr<class UImage> Image_ObjectIcon;
};
