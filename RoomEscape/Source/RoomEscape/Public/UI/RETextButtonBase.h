// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "RETextButtonBase.generated.h"

class UCommonTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonClickedSignature);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URETextButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;

protected:
	virtual void NativeOnCurrentTextStyleChanged() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UCommonTextBlock> TextBlock_ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter = GetButtonText, Setter = SetButtonText, meta = (AllowPrivateAccess = "true"), Category = "Appearance")
	FText ButtonText;

public:
	const FText& GetButtonText() const { return ButtonText; }

	void SetButtonText(const FText& NewButtonText);
};
