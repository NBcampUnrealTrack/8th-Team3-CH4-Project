// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "REClueWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UREClueWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
protected:
	// 단서 이름을 나타내는 TextBlock
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextBlock_ClueName;

	// 단서 내용을 보관하는 ScrollBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UScrollBox> ScrollBox_ClueContents;

	// UI에 표시되는 텍스트의 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FSlateColor Color_TextContents;

public:
	UFUNCTION(BlueprintCallable)
	FText GetClueName() const;

	UFUNCTION(BlueprintCallable)
	void SetClueName(FText ClueName);

	UFUNCTION(BlueprintCallable)
	void SetClueTextContents(TArray<FString> TextClues);

	UFUNCTION(BlueprintCallable)
	void AddClueTextContent(FString TextClue);

	UFUNCTION(BlueprintCallable)
	void SetClueImageContent(TSoftObjectPtr<UTexture2D> TextureClue);
};
