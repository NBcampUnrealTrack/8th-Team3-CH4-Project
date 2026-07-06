#include "Puzzles/BombDefusal/REBombDevice.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/BombDefusal/REBombDefusalManager.h"

AREBombDevice::AREBombDevice()
{
	bReplicates = true;

	DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
	DeviceMesh->SetupAttachment(SceneRoot);
	DeviceMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
	TimerText->SetupAttachment(SceneRoot);
	TimerText->SetHorizontalAlignment(EHTA_Center);
	TimerText->SetVerticalAlignment(EVRTA_TextCenter);
	TimerText->SetRelativeLocation(FVector(0.0, 0.0, 60.0));
	TimerText->SetText(FText::FromString(TEXT("--:--")));

	StepText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StepText"));
	StepText->SetupAttachment(SceneRoot);
	StepText->SetHorizontalAlignment(EHTA_Center);
	StepText->SetVerticalAlignment(EVRTA_TextCenter);
	StepText->SetRelativeLocation(FVector(0.0, 0.0, 40.0));
	StepText->SetText(FText::FromString(TEXT("0/0")));
}

void AREBombDevice::BeginPlay()
{
	Super::BeginPlay();

	if (AREBombDefusalManager* BombManager = GetBombManager())
	{
		BombManager->RegisterDevice(this);
	}
	RefreshNativeText();
}

void AREBombDevice::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DisplayRemainingTimeSeconds);
	DOREPLIFETIME(ThisClass, DisplayTimeLimitSeconds);
	DOREPLIFETIME(ThisClass, DisplayCurrentStepIndex);
	DOREPLIFETIME(ThisClass, DisplayTotalStepCount);
}

void AREBombDevice::SetBombManager(AREBombDefusalManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREBombDefusalManager* AREBombDevice::GetBombManager() const
{
	return Cast<AREBombDefusalManager>(PuzzleManager);
}

void AREBombDevice::ApplyServerTime(float RemainingTimeSeconds, float TimeLimitSeconds)
{
	if (HasAuthority() == false)
	{
		return;
	}

	DisplayRemainingTimeSeconds = RemainingTimeSeconds;
	DisplayTimeLimitSeconds = TimeLimitSeconds;
	OnRep_DeviceDisplay();
}

void AREBombDevice::ApplyServerStep(int32 CurrentStepIndex, int32 TotalStepCount)
{
	if (HasAuthority() == false)
	{
		return;
	}

	DisplayCurrentStepIndex = CurrentStepIndex;
	DisplayTotalStepCount = TotalStepCount;
	OnRep_DeviceDisplay();
}

void AREBombDevice::OnRep_DeviceDisplay()
{
	RefreshNativeText();
	ReceiveDeviceDisplayChanged(DisplayRemainingTimeSeconds, DisplayTimeLimitSeconds, DisplayCurrentStepIndex, DisplayTotalStepCount);
}

void AREBombDevice::RefreshNativeText()
{
	const int32 TotalSeconds = FMath::CeilToInt(DisplayRemainingTimeSeconds);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	if (IsValid(TimerText) == true)
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}

	if (IsValid(StepText) == true)
	{
		StepText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::Min(DisplayCurrentStepIndex + 1, DisplayTotalStepCount), DisplayTotalStepCount)));
	}
}
