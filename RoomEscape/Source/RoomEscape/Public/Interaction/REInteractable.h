// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "REInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UREInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOMESCAPE_API IREInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 상호작용 실행. 서버에서 호출된다. C++/BP 양쪽에서 구현 가능.
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* Interactor);
};
