// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REWidgetManager.h"

bool UREWidgetManager::ShouldCreateSubsystem(UObject* Outer) const
{
	Super::ShouldCreateSubsystem(Outer);
	return false;
}
