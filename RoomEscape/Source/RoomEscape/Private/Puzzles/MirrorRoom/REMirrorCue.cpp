#include "Puzzles/MirrorRoom/REMirrorCue.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Puzzles/MirrorRoom/REMirrorPuzzleData.h"
#include "Puzzles/MirrorRoom/REMirrorRoomManager.h"

AREMirrorCue::AREMirrorCue()
{
	CueRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CueRoot"));
	CueRoot->SetupAttachment(SceneRoot);

	CueMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CueMesh"));
	CueMesh->SetupAttachment(CueRoot);
	CueMesh->SetCollisionProfileName(TEXT("BlockAll"));

	CueTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CueTextRender"));
	CueTextRender->SetupAttachment(CueRoot);
	CueTextRender->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CueTextRender->SetHorizontalAlignment(EHTA_Center);
	CueTextRender->SetVerticalAlignment(EVRTA_TextCenter);
	CueTextRender->SetTextRenderColor(FColor::White);
	CueTextRender->SetWorldSize(TextWorldSize);
	CueTextRender->SetText(FallbackDisplayText);
}

void AREMirrorCue::BeginPlay()
{
	Super::BeginPlay();

	if (AREMirrorRoomManager* MirrorManager = GetMirrorManager())
	{
		MirrorManager->RegisterCue(this);
	}

	RefreshCueVisual();
}

void AREMirrorCue::SetMirrorManager(AREMirrorRoomManager* InManager)
{
	SetPuzzleManager(InManager);
}

AREMirrorRoomManager* AREMirrorCue::GetMirrorManager() const
{
	return Cast<AREMirrorRoomManager>(PuzzleManager);
}

UREMirrorPuzzleData* AREMirrorCue::GetResolvedPuzzleData() const
{
	if (IsValid(OverridePuzzleData) == true)
	{
		return OverridePuzzleData;
	}

	const AREMirrorRoomManager* MirrorManager = GetMirrorManager();
	return IsValid(MirrorManager) == true ? MirrorManager->GetMirrorPuzzleData() : nullptr;
}

void AREMirrorCue::RefreshCueVisual()
{
	FREMirrorCueDefinition CueDefinition;
	FText DisplayText = FallbackDisplayText;
	if (ResolveCueDefinition(CueDefinition) == true)
	{
		DisplayText = GetResolvedPuzzleData()->BuildCueDisplayText(CueDefinition);
	}

	if (IsValid(CueTextRender) == true)
	{
		CueTextRender->SetWorldSize(TextWorldSize);
		CueTextRender->SetText(DisplayText);
	}

	ReceiveCueVisualRefreshed(CueDefinition, DisplayText);
}

bool AREMirrorCue::ResolveCueDefinition(FREMirrorCueDefinition& OutCueDefinition) const
{
	const UREMirrorPuzzleData* ResolvedData = GetResolvedPuzzleData();
	if (IsValid(ResolvedData) == false)
	{
		return false;
	}

	if (CueId.IsNone() == false && ResolvedData->GetCueById(CueId, OutCueDefinition) == true)
	{
		return true;
	}

	return ResolvedData->GetCueByIndex(CueIndex, OutCueDefinition);
}
