// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/REClueWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"

FText UREClueWidget::GetClueName() const
{
	if (IsValid(TextBlock_ClueName) == true)
	{
		return TextBlock_ClueName->GetText();
	}
	return FText::GetEmpty();
}

void UREClueWidget::SetClueName(FText ClueName)
{
	if (IsValid(TextBlock_ClueName) == true)
	{
		TextBlock_ClueName->SetText(ClueName);
	}
	return;
}

void UREClueWidget::SetClueTextContents(TArray<FString> TextClues)
{
	if (IsValid(ScrollBox_ClueContents) == false)
	{
		return;
	}

	// 이미 존재하는 단서 TextBlock의 개수 구하기
	int32 ExistedContentsCount = ScrollBox_ClueContents->GetChildrenCount();

	for (int32 i = 0; i < ExistedContentsCount; ++i)
	{
		// 설정된 단서 내용
		const FString& ClueString = TextClues[i];

		// 단서 내용을 표시할 TextBlock
		UTextBlock* TextBlock_Clue = nullptr;

		// 해당 인덱스에 존재하는 TextBlock의 Instance 얻기
		if (i < ExistedContentsCount)
		{
			UWidget* ChildWidget = ScrollBox_ClueContents->GetChildAt(i);
			TextBlock_Clue = Cast<UTextBlock>(ChildWidget);
		}

		// TextBlock이 유효하지 않으면 생성
		if (IsValid(TextBlock_Clue) == false)
		{
			TextBlock_Clue = WidgetTree->ConstructWidget<UTextBlock>();
		}

		// TextBlock 내용 설정
		TextBlock_Clue->SetText(FText::FromString(ClueString));

		// TextBlock 색상 설정
		TextBlock_Clue->SetColorAndOpacity(Color_TextContents);

		TextBlock_Clue->SetAutoWrapText(true);

		// ScrollBox에 TextBlock 추가
		// 해당 인덱스에 존재하는 TextBlock의 Instance 얻기
		if (i >= ExistedContentsCount)
		{
			ScrollBox_ClueContents->AddChild(TextBlock_Clue);
		}
	}
}

void UREClueWidget::AddClueTextContent(FString TextClue)
{
	if (IsValid(ScrollBox_ClueContents) == false)
	{
		return;
	}

	// TextBlock 생성
	UTextBlock* TextBlock_Clue = WidgetTree->ConstructWidget<UTextBlock>();
	if (IsValid(TextBlock_Clue) == false)
	{
		return;
	}

	// TextBlock 내용 설정
	TextBlock_Clue->SetText(FText::FromString(TextClue));

	// TextBlock 색상 설정
	TextBlock_Clue->SetColorAndOpacity(Color_TextContents);

	TextBlock_Clue->SetAutoWrapText(true);

	// ScrollBox에 TextBlock 추가
	ScrollBox_ClueContents->AddChild(TextBlock_Clue);
}

void UREClueWidget::SetClueImageContent(TSoftObjectPtr<UTexture2D> TextureClue)
{
	if (IsValid(ScrollBox_ClueContents) == false)
	{
		return;
	}

	// ScrollBox의 자식 콘텐츠 삭제
	ScrollBox_ClueContents->ClearChildren();

	// Image 생성
	UImage* Image_Clue = WidgetTree->ConstructWidget<UImage>();
	if (IsValid(Image_Clue) == false)
	{
		return;
	}

	// 이미지 설정
	Image_Clue->SetBrushFromSoftTexture(TextureClue);

	// ScrollBox에 Widget 추가
	ScrollBox_ClueContents->AddChild(Image_Clue);
}
