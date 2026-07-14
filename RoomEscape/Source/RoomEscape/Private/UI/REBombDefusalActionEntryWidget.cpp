#include "UI/REBombDefusalActionEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"
#include "UI/REBombDefusalWidget.h"

void UREBombDefusalActionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureNativeFallbackLayout();
	ResolveWidgets();
	BindButtonEvent();
	RefreshActionState();
}

void UREBombDefusalActionEntryWidget::InitializeWireAction(UREBombDefusalWidget* InOwnerWidget, const FREBombWireDefinition& WireDefinition)
{
	OwnerBombWidget = InOwnerWidget;
	ActionType = EREBombActionType::Wire;
	WireIndex = WireDefinition.WireIndex;
	ButtonId = NAME_None;
	AccentColor = WireDefinition.UIAccentColor;

	if (WireDefinition.WireLabel.IsEmpty() == false)
	{
		ActionLabel = WireDefinition.WireLabel;
	}
	else
	{
		const UEnum* WireColorEnum = StaticEnum<EREBombWireColor>();
		const FText ColorText = WireColorEnum != nullptr
			? WireColorEnum->GetDisplayNameTextByValue(static_cast<int64>(WireDefinition.WireColor))
			: FText::FromString(TEXT("Wire"));
		ActionLabel = FText::Format(FText::FromString(TEXT("{0} Wire")), ColorText);
	}

	EnsureNativeFallbackLayout();
	ResolveWidgets();
	BindButtonEvent();
	ReceiveActionInitialized(ActionType, ActionLabel, AccentColor);
	RefreshActionState();
}

void UREBombDefusalActionEntryWidget::InitializeButtonAction(UREBombDefusalWidget* InOwnerWidget, const FREBombButtonDefinition& ButtonDefinition)
{
	OwnerBombWidget = InOwnerWidget;
	ActionType = EREBombActionType::Button;
	WireIndex = INDEX_NONE;
	ButtonId = ButtonDefinition.ButtonId;
	AccentColor = ButtonDefinition.UIAccentColor;
	ActionLabel = ButtonDefinition.ButtonLabel.IsEmpty() == false
		? ButtonDefinition.ButtonLabel
		: FText::FromString(ButtonDefinition.ButtonId.ToString());

	EnsureNativeFallbackLayout();
	ResolveWidgets();
	BindButtonEvent();
	ReceiveActionInitialized(ActionType, ActionLabel, AccentColor);
	RefreshActionState();
}

void UREBombDefusalActionEntryWidget::RequestAction()
{
	if (IsValid(OwnerBombWidget) == false)
	{
		return;
	}

	if (ActionType == EREBombActionType::Wire)
	{
		OwnerBombWidget->RequestCutWire(WireIndex);
	}
	else
	{
		OwnerBombWidget->RequestToggleButton(ButtonId);
	}
}

void UREBombDefusalActionEntryWidget::RefreshActionState()
{
	AREBombDefusalManager* BombManager = IsValid(OwnerBombWidget) == true ? OwnerBombWidget->GetBombManager() : nullptr;
	const bool bCanSubmitBase = IsValid(OwnerBombWidget) == true && OwnerBombWidget->CanSubmitActions() == true;

	if (ActionType == EREBombActionType::Wire)
	{
		bCurrentState = IsValid(BombManager) == true && BombManager->IsWireCut(WireIndex) == true;
	}
	else
	{
		bCurrentState = IsValid(BombManager) == true && BombManager->IsButtonPressed(ButtonId) == true;
	}

	const bool bCanSubmit = bCanSubmitBase == true && (ActionType != EREBombActionType::Wire || bCurrentState == false);
	if (IsValid(BTN_Action) == true)
	{
		BTN_Action->SetIsEnabled(bCanSubmit);
	}

	ApplyVisualState(bCanSubmit);
	ReceiveActionStateChanged(bCurrentState, bCanSubmit);
}

EREBombActionType UREBombDefusalActionEntryWidget::GetActionType() const
{
	return ActionType;
}

int32 UREBombDefusalActionEntryWidget::GetWireIndex() const
{
	return WireIndex;
}

FName UREBombDefusalActionEntryWidget::GetButtonId() const
{
	return ButtonId;
}

void UREBombDefusalActionEntryWidget::HandleActionClicked()
{
	RequestAction();
}

void UREBombDefusalActionEntryWidget::EnsureNativeFallbackLayout()
{
	if (IsValid(WidgetTree) == false || IsValid(WidgetTree->RootWidget) == true)
	{
		return;
	}

	BTN_Action = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BTN_Action"));
	TXT_ActionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_ActionLabel"));
	if (IsValid(BTN_Action) == true && IsValid(TXT_ActionLabel) == true)
	{
		BTN_Action->AddChild(TXT_ActionLabel);
		WidgetTree->RootWidget = BTN_Action;
	}
}

void UREBombDefusalActionEntryWidget::ResolveWidgets()
{
	if (IsValid(WidgetTree) == false)
	{
		return;
	}

	if (IsValid(BTN_Action) == false)
	{
		BTN_Action = Cast<UButton>(WidgetTree->FindWidget(TEXT("BTN_Action")));
	}

	if (IsValid(TXT_ActionLabel) == false)
	{
		TXT_ActionLabel = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ActionLabel")));
	}

	if (IsValid(TXT_ActionState) == false)
	{
		TXT_ActionState = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("TXT_ActionState")));
	}

	if (IsValid(IMG_Accent) == false)
	{
		IMG_Accent = Cast<UImage>(WidgetTree->FindWidget(TEXT("IMG_Accent")));
	}
}

void UREBombDefusalActionEntryWidget::BindButtonEvent()
{
	if (IsValid(BTN_Action) == true)
	{
		BTN_Action->OnClicked.RemoveAll(this);
		BTN_Action->OnClicked.AddDynamic(this, &ThisClass::HandleActionClicked);
	}
}

void UREBombDefusalActionEntryWidget::ApplyVisualState(bool bCanSubmit)
{
	const FText StateText = ActionType == EREBombActionType::Wire
		? (bCurrentState == true ? FText::FromString(TEXT("잘림")) : FText::FromString(TEXT("대기")))
		: (bCurrentState == true ? FText::FromString(TEXT("눌림")) : FText::FromString(TEXT("해제")));

	if (IsValid(IMG_Accent) == true)
	{
		IMG_Accent->SetColorAndOpacity(AccentColor);
	}

	if (IsValid(TXT_ActionState) == true)
	{
		TXT_ActionState->SetText(StateText);
	}

	if (IsValid(TXT_ActionLabel) == true)
	{
		if (IsValid(TXT_ActionState) == true)
		{
			TXT_ActionLabel->SetText(ActionLabel);
		}
		else
		{
			TXT_ActionLabel->SetText(FText::Format(
				FText::FromString(TEXT("{0}  [{1}]")),
				ActionLabel,
				StateText));
		}
	}

	SetIsEnabled(bCanSubmit);
}
