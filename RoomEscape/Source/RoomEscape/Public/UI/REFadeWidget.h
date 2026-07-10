#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "REFadeWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FREFadeRequestSignature, float, DurationSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FREFadeSimpleSignature);

UCLASS(Blueprintable)
class ROOMESCAPE_API UREFadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Fade")
	FREFadeRequestSignature OnFadeOutRequested;

	UPROPERTY(BlueprintAssignable, Category = "Fade")
	FREFadeRequestSignature OnFadeInRequested;

	UPROPERTY(BlueprintAssignable, Category = "Fade")
	FREFadeSimpleSignature OnFadeOutFinished;

	UPROPERTY(BlueprintAssignable, Category = "Fade")
	FREFadeSimpleSignature OnFadeInFinished;

public:
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void PlayFadeOut(float DurationSeconds);

	UFUNCTION(BlueprintCallable, Category = "Fade")
	void PlayFadeIn(float DurationSeconds);

	UFUNCTION(BlueprintCallable, Category = "Fade")
	void NotifyFadeOutFinished();

	UFUNCTION(BlueprintCallable, Category = "Fade")
	void NotifyFadeInFinished();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Fade")
	void ReceiveFadeOutRequested(float DurationSeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Fade")
	void ReceiveFadeInRequested(float DurationSeconds);
};
