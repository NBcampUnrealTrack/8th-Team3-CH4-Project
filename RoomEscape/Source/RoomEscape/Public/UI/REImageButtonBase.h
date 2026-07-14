// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "REImageButtonBase.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREImageButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout", meta = (BindWidget))
	TObjectPtr<class UImage> Image_Button;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", BlueprintSetter = SetButtonTexture, Setter = SetButtonTexture)
	TSoftObjectPtr<UTexture2D> Texture_Button;

public:
	UFUNCTION(BlueprintCallable)
	void SetButtonTexture(TSoftObjectPtr<UTexture2D> Texture);
};
