// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChatBoxWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/ComboBoxString.h"
#include "Components/RichTextBlock.h"
#include "GameplayTags.h"
#include "Components/ChattingComponent.h"

void UChatBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(EditableText_InputField) == true)
	{
		EditableText_InputField->OnTextCommitted.AddUniqueDynamic(this, &UChatBoxWidget::OnTextCommitted);
	}
}

void UChatBoxWidget::NativeDestruct()
{
	if (IsValid(EditableText_InputField) == true)
	{
		EditableText_InputField->OnTextCommitted.RemoveDynamic(this, &UChatBoxWidget::OnTextCommitted);
	}

	Super::NativeDestruct();
}

void UChatBoxWidget::InitializeWidgetByContextObject_Implementation(UObject* ContextObject)
{
}

void UChatBoxWidget::InitializeWidgetByComponent_Implementation(UActorComponent* Component)
{
	// 필요한 Component 변환 후 유효성 확인
	UChattingComponent* ChattingComponent = Cast<UChattingComponent>(Component);
	if (IsValid(ChattingComponent) == false)
	{
		return;
	}

	ChattingComponent->OnMessageReceived.AddDynamic(this, &UChatBoxWidget::OnMessageReceived);
}

void UChatBoxWidget::InitializeWidgetByActor_Implementation(AActor* Actor)
{
}

void UChatBoxWidget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Enter 키 입력을 제외한 메시지 전송 금지
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	// Message를 FString으로 변환
	FString Message = Text.ToString();

	// Message의 양끝에 존재하는 공백 제거
	Message.TrimStartAndEndInline();

	// 비어있는 메시지의 경우 함수 조기 종료 (전송하지 않음)
	if (Message.IsEmpty() == true)
	{
		return;
	}

	// 메시지 입력 필드 초기화
	EditableText_InputField->SetText(FText::GetEmpty());

	// 채널 태그 얻기
	FGameplayTag ChannelTag = FGameplayTag::EmptyTag;
	if (IsValid(ComboBox_ChannelSelector) == true)
	{
		FString TagString = ComboBox_ChannelSelector->GetSelectedOption();
		ChannelTag = FGameplayTag::RequestGameplayTag(FName(TagString), false);
	}

	// 메시지 전송 이벤트 실행
	OnMessageCommitted.Broadcast(ChannelTag, Message);
}

void UChatBoxWidget::OnMessageReceived(const FGameplayTag& ChannelTag, const FString& Sender, const FString& Message)
{
	// RichTextBlock을 생성하기 위한 WidgetTree와 ChatLog의 스타일을 정의한 DataTable의 유효성 검사
	if (IsValid(WidgetTree) == false || IsValid(ChatLogTextStyleTable) == false)
	{
		return;
	}
	
	// 채팅 로그를 저장하는 ScrollBox의 유효성 검사
	if (IsValid(ScrollBox_LogContainer) == false)
	{
		return;
	}

	// RichTextBlock 생성 및 유효성 검사
	URichTextBlock* RichTextBlock_ChatLog = WidgetTree->ConstructWidget<URichTextBlock>();
	if (IsValid(RichTextBlock_ChatLog) == false)
	{
		return;
	}

	// RichTextBlock의 TextStyleSet 설정
	RichTextBlock_ChatLog->SetTextStyleSet(ChatLogTextStyleTable);

	// 수신된 메시지를 RichText 스타일으로 변환
	FString RichTextStyleMessageString = FString::Printf(TEXT("[%s] %s : %s"), *FDateTime::Now().ToFormattedString(TEXT("%H:%M")), *Sender, *Message);

	// TextBlock의 Text 설정
	RichTextBlock_ChatLog->SetText(FText::FromString(RichTextStyleMessageString));

	// ScrollBox의 자식으로 RichTextBlock 추가
	ScrollBox_LogContainer->AddChild(RichTextBlock_ChatLog);
}
