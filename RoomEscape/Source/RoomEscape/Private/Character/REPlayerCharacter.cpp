// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/REPlayerCharacter.h"
#include "AbilitySystem/Abilities/REGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/Abilities/GA_Interact.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/REInteractable.h"


// Sets default values
AREPlayerCharacter::AREPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// --- 카메라 : 캡슐 부착, 눈높이, 마우스 Look으로 회전 제어
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f)); // 눈 높이
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	// --- 1인칭 팔 : 카메라 북착, 소유자에게만 보임
	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
	FirstPersonArms->SetupAttachment(FirstPersonCamera);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->bCastDynamicShadow = false;
	FirstPersonArms->CastShadow = false;
	
	// ---전신 : 내 화면엔 숨기고 상대에게만 보임(상대에게 보일 필요 없으면 삭제 요망)
	GetMesh()->SetOwnerNoSee(true);
	
	// --- 1인칭 회전
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// --- ASC 생성
	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
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

	// 서버 검증: 거리 체크(클라가 엉뚱한 대상 보내는 것 방지)
	const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Dist > InteractionDistance + 100.f)   // 약간의 여유
	{
		return;
	}

	// 서버 권한으로 실제 상호작용
	IREInteractable::Execute_Interact(Target, this);
	UE_LOG(LogTemp, Warning, TEXT("[ServerInteract] %s -> %s"), *GetName(), *Target->GetName());
}

bool AREPlayerCharacter::ServerInteract_Validate(AActor* Target)
{
	return true;
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
	}
}

void AREPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 서버 경로
	InitAbilityActorInfo();
	GrantDefaultAbilities();
}

void AREPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 원경 클라 경로
	InitAbilityActorInfo();
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
		// 시선(컨트롤 회전) 기준 전/후·좌/우
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, MoveVector.Y);
		AddMovementInput(Right,   MoveVector.X);
	}
}

void AREPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AREPlayerCharacter::InitAbilityActorInfo()
{
	if (AbilitySystemComp)
	{
		// OwnerActor, AvatarActor = 자기자신
		AbilitySystemComp->InitAbilityActorInfo(this, this);
		
		// 임시로그
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Red,
			FString::Printf(TEXT("[ASC Init] %s Auth=%d Local=%d"), 
				*GetName(), HasAuthority(), IsLocallyControlled()));
		UE_LOG(LogTemp, Warning, TEXT("[ASC Init] %s | Authority=%d | LocallyControlled=%d | Role=%d"),
				*GetName(),
				HasAuthority(),
				IsLocallyControlled(),
				(int32)GetLocalRole());
	}
}

void AREPlayerCharacter::GrantDefaultAbilities()
{
	// 어빌리티 부여는 서버에서만. 부여 결과는 클라로 복제됨
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
