#include "UI/REFadeWidget.h"

void UREFadeWidget::PlayFadeOut(float DurationSeconds)
{
	const float SanitizedDuration = FMath::Max(0.0f, DurationSeconds);
	OnFadeOutRequested.Broadcast(SanitizedDuration);
	ReceiveFadeOutRequested(SanitizedDuration);
}

void UREFadeWidget::PlayFadeIn(float DurationSeconds)
{
	const float SanitizedDuration = FMath::Max(0.0f, DurationSeconds);
	OnFadeInRequested.Broadcast(SanitizedDuration);
	ReceiveFadeInRequested(SanitizedDuration);
}

void UREFadeWidget::NotifyFadeOutFinished()
{
	OnFadeOutFinished.Broadcast();
}

void UREFadeWidget::NotifyFadeInFinished()
{
	OnFadeInFinished.Broadcast();
}
