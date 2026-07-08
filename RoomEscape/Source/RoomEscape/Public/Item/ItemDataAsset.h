// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(AssetType, ItemName);
	}

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults")
	FPrimaryAssetType AssetType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults")
	FName ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AssetBundles = "UI"))
	TObjectPtr<UTexture2D> Texture_Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AssetBundles = "World"))
	TSubclassOf<AActor> ItemClass;
};
