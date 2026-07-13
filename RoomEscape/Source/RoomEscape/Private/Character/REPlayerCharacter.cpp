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
#include "Game/RENotifySubsystem.h"
#include "UI/LocalWidgetManager.h"
#include "UI/RERootCanvasWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

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

	UE_LOG(LogTemp, Warning, TEXT("[ServerInteract] %s -> %s"), *GetName(), *Target->GetName());
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
	if (AbilitySystemComp)
	{
		AbilitySystemComp->TryActivateAbilityByClass(UGA_Flashlight::StaticClass());
	}
}

void AREPlayerCharacter::Input_ToggleInventory()
{
	URENotifySubsystem* NotifySubsystem = URENotifySubsystem::GetInstance(this);

	ULocalWidgetManager* WidgetManager = ULocalWidgetManager::GetInstance(this);
	if (IsValid(WidgetManager) == false)
	{
		if (IsValid(NotifySubsystem) == true)
		{
			NotifySubsystem->NotifyEvent(RETag::Event::Debug::Test, TEXT("ToggleInventory - WidgetManager 없음"));
		}
		return;
	}

	UWidget* InventoryWidget = WidgetManager->FindWidget(FName("Inventory"));
	if (IsValid(InventoryWidget) == false)
	{
		if (IsValid(NotifySubsystem) == true)
		{
			NotifySubsystem->NotifyEvent(RETag::Event::Debug::Test, TEXT("ToggleInventory - Inventory 위젯 미등록"));
		}
		return;
	}

	const bool bNewHidden = InventoryWidget->GetVisibility() != ESlateVisibility::Collapsed;
	WidgetManager->SetWidgetHiddenInGame(FName("Inventory"), bNewHidden);

	if (IsValid(NotifySubsystem) == true)
	{
		NotifySubsystem->NotifyEvent(RETag::Event::Debug::Test, FString::Printf(TEXT("ToggleInventory - Hidden=%d"), bNewHidden));
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

void AREPlayerCharacter::ApplyFlashlightVisual()
{
	if (FlashlightComponent)
	{
		FlashlightComponent->SetVisibility(bFlashlightOn);
	}
}
