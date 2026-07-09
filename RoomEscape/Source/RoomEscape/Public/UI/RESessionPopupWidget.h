// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/REPopupWidget.h"
#include "RESessionPopupWidget.generated.h"

class UEditableTextBox;

/**
 * 
 */
UCLASS()
class ROOMESCAPE_API URESessionPopupWidget : public UREPopupWidget
{
	GENERATED_BODY()
	
protected:
	// Session의 이름 또는 태그를 입력받는 EditableTextBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UEditableTextBox> InputField_SessionName;

	// Session의 비밀번호를 입력받는 EditableTextBox
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget), Category = "Layout")
	TObjectPtr<UEditableTextBox> InputField_SessionPassword;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	TSubclassOf<UCommonActivatableWidget> SessionRoomWidgetClass;

public:
	UFUNCTION(BlueprintCallable)
	void AddPopupButton_HostGame();

	UFUNCTION(BlueprintCallable)
	void HostGameSession();

	UFUNCTION(BlueprintCallable)
	void AddPopupButton_JoinGame();

	UFUNCTION(BlueprintCallable)
	void JoinGameSession();

protected:
	UFUNCTION(BlueprintCallable)
	void OnCreateOrJoinSessionRoom();
};
