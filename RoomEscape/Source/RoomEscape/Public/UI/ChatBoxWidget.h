// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InitializeUtilityInterface.h"
#include "ChatBoxWidget.generated.h"

// 메시지를 전송하려는 경우 
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageCommittedSignature, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageCommittedSignature, const struct FGameplayTag&, ChannelTag, const FString&, Message);

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API UChatBoxWidget : public UUserWidget, public IInitializeUtilityInterface
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnMessageCommittedSignature OnMessageCommitted;

protected:
#pragma region Widget Components

	// 채팅 UI 크기를 조절하는 SizeBox
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<class USizeBox> SizeBox_Root;

	// 채팅 로그 RichTextBlock을 관리하는 ScrollBox
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UScrollBox> ScrollBox_LogContainer;

	// 채팅 채널을 선택할 수 있는 ComboBox
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<class UComboBoxString> ComboBox_ChannelSelector;

	// 전송할 채팅 메시지를 입력받는 EditableText
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UEditableText> EditableText_InputField;

#pragma endregion

#pragma region Properties

	// 채팅 로그의 표시되는 Style을 정의한 DataTable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", RequiredAssetDataTags = "RowStructure=/Script/UMG.RichTextStyleRow"))
	TObjectPtr<UDataTable> ChatLogTextStyleTable;

#pragma endregion

protected:
#pragma region Derived from IInitializeUtilityInterface

	void InitializeWidgetByContextObject_Implementation(UObject* ContextObject) override;
	void InitializeWidgetByComponent_Implementation(UActorComponent* Component) override;
	void InitializeWidgetByActor_Implementation(AActor* Actor) override;

#pragma endregion

	// InputField에서 Text 입력이 완료되었을 경우 호출되는 함수
	// UEditableText::OnTextCommitted 이벤트에 연결됨
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// 메시지를 수신하는 경우 ScrollBox에 RichTextBlock으로 그 내용을 표시하는 함수
	// ChannelTag : 메시지를 수신한 채널
	// Sender : 메시지를 전송한 클라이언트의 이름(닉네임)
	// Message : 수신된 메시지 내용
	UFUNCTION()
	void OnMessageReceived(const FGameplayTag& ChannelTag, const FString& Sender, const FString& Message);
};
