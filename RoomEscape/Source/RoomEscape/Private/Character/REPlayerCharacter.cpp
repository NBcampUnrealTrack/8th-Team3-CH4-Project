#include "Character/REPlayerCharacter.h"
#include "AbilitySystem/Abilities/REGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/GA_Interact.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "AbilitySystem/NativeGameplayTags.h"
#include "AbilitySystem/Abilities/GA_Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "Interaction/REInteractable.h"
#include "Net/UnrealNetwork.h"
#include "Puzzles/Framework/REPuzzleInteractableActor.h"
#include "Components/REInventoryComponent.h"
#include "UI/LocalWidgetManager.h"
#include "UI/RERootCanvasWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "TimerManager.h"

AREPlayerCharacter::AREPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
	FirstPersonArms->SetupAttachment(FirstPersonCamera);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->bCastDynamicShadow = false;
	FirstPersonArms->CastShadow = false;

	FlashlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightComponent"));
	FlashlightComponent->SetupAttachment(FirstPersonCamera);
	FlashlightComponent->SetVisibility(false);
	FlashlightComponent->Intensity = 5000.f;
	FlashlightComponent->OuterConeAngle = 25.f;
	FlashlightComponent->AttenuationRadius = 1500.f;
	FlashlightComponent->CastShadows = false;

	GetMesh()->SetOwnerNoSee(true);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	JumpMaxCount = 1;
	JumpMaxHoldTime = 0.12f;
	ApplyJumpMovementSettings();

	NativeJumpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
	if (NativeJumpAction)
	{
		NativeJumpAction->ValueType = EInputActionValueType::Boolean;
	}

	InventoryComponent = CreateDefaultSubobject<UREInventoryComponent>(TEXT("InventoryComponent"));

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AREPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyJumpMovementSettings();
	CacheFlashlightRelativeTransform();
	ApplyFlashlightVisual();
}

void AREPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// 로컬 플레이어에서만 호출되지만 방어적으로 확인
	if (IsLocallyControlled() == false)
	{
		return;
	}

	PushHUDWidget();

	// 인벤토리 Widget 초기화 - BeginPlay 시점에는 클라이언트의 Owner 복제가
	// 완료되지 않을 수 있으므로 로컬 컨트롤러가 보장되는 이 시점에 호출
	if (IsValid(InventoryComponent) == true)
	{
		IWidgetInitializableInterface::Execute_InitWidget(InventoryComponent);
	}
}

void AREPlayerCharacter::PushHUDWidget()
{
	if (IsValid(HUDWidgetClass) == false)
	{
		return;
	}

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	// 리스폰/재접속 시 중복 Push 방지
	if (IsValid(WidgetManager->FindWidget(FName("HUD"))) == true)
	{
		return;
	}

	URERootCanvasWidget* RootCanvasWidget = Cast<URERootCanvasWidget>(WidgetManager->GetRootWidget());
	if (IsValid(RootCanvasWidget) == false)
	{
		return;
	}

	UCommonActivatableWidgetStack* PrimaryLayer = RootCanvasWidget->GetPrimaryWidgetStack();
	if (IsValid(PrimaryLayer) == false)
	{
		return;
	}

	UCommonActivatableWidget* HUDInstance = PrimaryLayer->AddWidget<UCommonActivatableWidget>(HUDWidgetClass);
	if (IsValid(HUDInstance) == true)
	{
		// 다른 컴포넌트(Timer, Chatting 등)가 RequestAsync("HUD")로 접근할 수 있도록 등록
		WidgetManager->AddWidgetInstance(FName("HUD"), HUDInstance);
	}
}

UAbilitySystemComponent* AREPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

bool AREPlayerCharacter::GetFlashlightBeamData(
	FVector& OutOrigin,
	FVector& OutDirection,
	float& OutRange,
	float& OutInnerConeAngle,
	float& OutOuterConeAngle) const
{
	if (IsFlashlightOn() == false || IsValid(FlashlightComponent) == false)
	{
		return false;
	}

	const FRotator AimRotation = GetBaseAimRotation();
	const FVector ViewOrigin = IsValid(FirstPersonCamera) == true
		? FirstPersonCamera->GetComponentLocation()
		: GetPawnViewLocation();
	const FVector RelativeLocation = bHasCachedFlashlightRelativeTransform == true
		? CachedFlashlightRelativeLocation
		: FlashlightComponent->GetRelativeLocation();
	const FRotator RelativeRotation = bHasCachedFlashlightRelativeTransform == true
		? CachedFlashlightRelativeRotation
		: FlashlightComponent->GetRelativeRotation();

	OutOrigin = ViewOrigin + AimRotation.RotateVector(RelativeLocation);
	OutDirection = AimRotation.RotateVector(RelativeRotation.Vector()).GetSafeNormal();
	OutRange = FMath::Max(FlashlightComponent->AttenuationRadius, 0.0f);
	OutInnerConeAngle = FMath::Clamp(FlashlightComponent->InnerConeAngle, 0.0f, 89.0f);
	OutOuterConeAngle = FMath::Clamp(FlashlightComponent->OuterConeAngle, OutInnerConeAngle, 89.0f);

	return OutRange > KINDA_SMALL_NUMBER && OutDirection.IsNearlyZero() == false;
}

void AREPlayerCharacter::ServerInteract_Implementation(AActor* Target)
{
	if (!Target || !Target->Implements<UREInteractable>())
	{
		return;
	}

	const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Dist > InteractionDistance + 100.f)
	{
		return;
	}

	if (AREPuzzleInteractableActor* PuzzleInteractable = Cast<AREPuzzleInteractableActor>(Target))
	{
		PuzzleInteractable->ProcessServerInteract(this);
	}
	else
	{
		IREInteractable::Execute_Interact(Target, this);
	}

	UE_LOG(LogTemp, Log,
		TEXT("[ServerInteract] Dispatched request: %s -> %s. Target acceptance/rejection is logged by the target actor."),
		*GetName(),
		*Target->GetName());
}

bool AREPlayerCharacter::ServerInteract_Validate(AActor* Target)
{
	return true;
}

void AREPlayerCharacter::ServerToggleFlashlight_Implementation()
{
	bFlashlightOn = !bFlashlightOn;
	ApplyFlashlightVisual();

	if (AbilitySystemComp)
	{
		if (bFlashlightOn)
		{
			AbilitySystemComp->AddLooseGameplayTag(RETag::State::Flashlight::On);
		}
		else
		{
			AbilitySystemComp->RemoveLooseGameplayTag(RETag::State::Flashlight::On);
		}
	}

	ForceNetUpdate();
}

bool AREPlayerCharacter::ServerToggleFlashlight_Validate()
{
	return true;
}

bool AREPlayerCharacter::IsFlashlightOn() const
{
	return bFlashlightOn;
}

AActor* AREPlayerCharacter::TraceForInteractable(FHitResult& OutHit) const
{
	if (!FirstPersonCamera)
	{
		return nullptr;
	}

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractionDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		return OutHit.GetActor();
	}
	return nullptr;
}

void AREPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AREPlayerCharacter::Input_Interact);
		}
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AREPlayerCharacter::Input_Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AREPlayerCharacter::Input_Look);
		}
		if (UInputAction* EffectiveJumpAction = GetJumpInputAction())
		{
			EIC->BindAction(EffectiveJumpAction, ETriggerEvent::Started, this, &AREPlayerCharacter::Input_JumpStarted);
			EIC->BindAction(EffectiveJumpAction, ETriggerEvent::Completed, this, &AREPlayerCharacter::Input_JumpCompleted);
			EIC->BindAction(EffectiveJumpAction, ETriggerEvent::Canceled, this, &AREPlayerCharacter::Input_JumpCompleted);
		}
		if (FlashlightAction)
		{
			EIC->BindAction(FlashlightAction, ETriggerEvent::Started, this, &AREPlayerCharacter::Input_Flashlight);
		}
		if (ToggleInventoryAction)
		{
			EIC->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AREPlayerCharacter::Input_ToggleInventory);
		}
	}

	RegisterJumpMappingContext();
}

void AREPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlashlightTransformTimerHandle);
	}

	UnregisterJumpMappingContext();
	Super::EndPlay(EndPlayReason);
}

void AREPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	GrantDefaultAbilities();
}

void AREPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AREPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AREPlayerCharacter, bFlashlightOn);
}

void AREPlayerCharacter::Input_Interact()
{
	// 인벤토리/퍼즐 위젯 등 UI가 입력을 점유 중이면 상호작용 차단 (점프와 동일 규약)
	if (Controller && Controller->IsMoveInputIgnored())
	{
		return;
	}

	if (AbilitySystemComp)
	{
		AbilitySystemComp->TryActivateAbilityByClass(UGA_Interact::StaticClass());
	}
}

void AREPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Forward, MoveVector.Y);
		AddMovementInput(Right, MoveVector.X);
	}
}

void AREPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AREPlayerCharacter::Input_JumpStarted()
{
	if (Controller && Controller->IsMoveInputIgnored())
	{
		return;
	}
	Jump();
}

void AREPlayerCharacter::Input_JumpCompleted()
{
	StopJumping();
}

void AREPlayerCharacter::Input_Flashlight()
{
	// UI가 입력을 점유 중이면 손전등 토글 차단
	if (Controller && Controller->IsMoveInputIgnored())
	{
		return;
	}

	if (AbilitySystemComp)
	{
		AbilitySystemComp->TryActivateAbilityByClass(UGA_Flashlight::StaticClass());
	}
}

void AREPlayerCharacter::Input_ToggleInventory()
{
	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		return;
	}

	UWidget* InventoryWidget = WidgetManager->FindWidget(FName("Inventory"));
	if (IsValid(InventoryWidget) == false)
	{
		return;
	}

	const bool bNewHidden = InventoryWidget->GetVisibility() != ESlateVisibility::Collapsed;

	// 열려는 시점에 이미 다른 UI(퍼즐 위젯 등)가 입력을 점유 중이면 열지 않음
	// (닫을 때는 인벤토리 자신이 점유 중이므로 통과되어야 함)
	if (bNewHidden == false && Controller && Controller->IsMoveInputIgnored())
	{
		return;
	}

	WidgetManager->SetWidgetHiddenInGame(FName("Inventory"), bNewHidden);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	if (bNewHidden == true)
	{
		// 인벤토리 닫힘 - 게임 입력 복원
		PlayerController->ResetIgnoreMoveInput();
		PlayerController->ResetIgnoreLookInput();
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->FlushPressedKeys();
	}
	else
	{
		// 인벤토리 열림 - 이동/시선 차단 + 마우스 커서 표시
		// 위젯에 포커스를 주지 않아야 토글 키(I) 입력이 계속 동작함
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
}

void AREPlayerCharacter::OnRep_FlashlightOn()
{
	ApplyFlashlightVisual();
}

void AREPlayerCharacter::InitAbilityActorInfo()
{
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("[ASC Init] %s Auth=%d Local=%d"), *GetName(), HasAuthority(), IsLocallyControlled()));
		UE_LOG(LogTemp, Warning, TEXT("[ASC Init] %s | Authority=%d | LocallyControlled=%d | Role=%d"), *GetName(), HasAuthority(), IsLocallyControlled(), (int32)GetLocalRole());
	}
}

void AREPlayerCharacter::GrantDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComp)
	{
		return;
	}

	for (const TSubclassOf<UREGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComp->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

void AREPlayerCharacter::ApplyJumpMovementSettings()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->JumpZVelocity = CharacterJumpZVelocity;
		MovementComponent->GravityScale = CharacterGravityScale;
		MovementComponent->AirControl = CharacterAirControl;
		MovementComponent->BrakingDecelerationFalling = CharacterBrakingDecelerationFalling;
	}
}

void AREPlayerCharacter::RegisterJumpMappingContext()
{
	if (bRegisterJumpMappingAtRuntime == false)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (IsValid(PlayerController) == false || PlayerController->IsLocalController() == false)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (IsValid(LocalPlayer) == false)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	UInputAction* EffectiveJumpAction = GetJumpInputAction();
	if (IsValid(Subsystem) == false || IsValid(EffectiveJumpAction) == false || JumpKey.IsValid() == false)
	{
		return;
	}

	if (IsValid(RuntimeJumpMappingContext) == false)
	{
		RuntimeJumpMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Jump_Runtime"));
		RuntimeJumpMappingContext->MapKey(EffectiveJumpAction, JumpKey);
	}

	Subsystem->RemoveMappingContext(RuntimeJumpMappingContext);
	Subsystem->AddMappingContext(RuntimeJumpMappingContext, JumpMappingPriority);
}

void AREPlayerCharacter::UnregisterJumpMappingContext()
{
	if (IsValid(RuntimeJumpMappingContext) == false)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (IsValid(PlayerController) == true && PlayerController->IsLocalController() == true)
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->RemoveMappingContext(RuntimeJumpMappingContext);
			}
		}
	}
}

UInputAction* AREPlayerCharacter::GetJumpInputAction() const
{
	return IsValid(JumpAction) == true ? JumpAction.Get() : NativeJumpAction.Get();
}

void AREPlayerCharacter::CacheFlashlightRelativeTransform()
{
	if (bHasCachedFlashlightRelativeTransform == true)
	{
		return;
	}

	if (IsValid(FlashlightComponent) == false)
	{
		bHasCachedFlashlightRelativeTransform = false;
		return;
	}

	CachedFlashlightRelativeLocation = FlashlightComponent->GetRelativeLocation();
	CachedFlashlightRelativeRotation = FlashlightComponent->GetRelativeRotation();
	bHasCachedFlashlightRelativeTransform = true;
}

void AREPlayerCharacter::ApplyFlashlightVisual()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == true)
	{
		World->GetTimerManager().ClearTimer(FlashlightTransformTimerHandle);
	}

	if (IsValid(FlashlightComponent) == false)
	{
		return;
	}

	CacheFlashlightRelativeTransform();
	FlashlightComponent->SetVisibility(bFlashlightOn);

	if (bFlashlightOn == false || GetNetMode() == NM_DedicatedServer || IsValid(World) == false)
	{
		return;
	}

	UpdateFlashlightTransform();
	World->GetTimerManager().SetTimer(
		FlashlightTransformTimerHandle,
		this,
		&ThisClass::UpdateFlashlightTransform,
		1.0f / 60.0f,
		true);
}

void AREPlayerCharacter::UpdateFlashlightTransform()
{
	if (IsValid(FlashlightComponent) == false)
	{
		return;
	}

	FVector BeamOrigin = FVector::ZeroVector;
	FVector BeamDirection = FVector::ForwardVector;
	float BeamRange = 0.0f;
	float InnerConeAngle = 0.0f;
	float OuterConeAngle = 0.0f;

	if (GetFlashlightBeamData(BeamOrigin, BeamDirection, BeamRange, InnerConeAngle, OuterConeAngle) == false)
	{
		return;
	}

	FlashlightComponent->SetWorldLocationAndRotation(BeamOrigin, BeamDirection.Rotation());
}
