#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputCoreTypes.h"
#include "REPlayerCharacter.generated.h"

class USpotLightComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UREGameplayAbility;
class UCameraComponent;
class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class ROOMESCAPE_API AREPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AREPlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerToggleFlashlight();

	UFUNCTION(BlueprintPure, Category="Flashlight")
	bool IsFlashlightOn() const;

	AActor* TraceForInteractable(FHitResult& OutHit) const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void Input_Interact();
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStarted();
	void Input_JumpCompleted();
	void Input_Flashlight();

	UFUNCTION()
	void OnRep_FlashlightOn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> FlashlightAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Jump")
	bool bRegisterJumpMappingAtRuntime = true;

	UPROPERTY(EditDefaultsOnly, Category="Input|Jump")
	FKey JumpKey = EKeys::SpaceBar;

	UPROPERTY(EditDefaultsOnly, Category="Input|Jump")
	int32 JumpMappingPriority = 1;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> NativeJumpAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RuntimeJumpMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractionDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Jump", meta = (ClampMin = "0.0"))
	float CharacterJumpZVelocity = 420.0f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Jump", meta = (ClampMin = "0.0"))
	float CharacterGravityScale = 1.55f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Jump", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CharacterAirControl = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category="Movement|Jump", meta = (ClampMin = "0.0"))
	float CharacterBrakingDecelerationFalling = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TArray<TSubclassOf<UREGameplayAbility>> DefaultAbilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flashlight")
	TObjectPtr<USpotLightComponent> FlashlightComponent;

	UPROPERTY(ReplicatedUsing=OnRep_FlashlightOn, VisibleAnywhere, BlueprintReadOnly, Category="Flashlight")
	bool bFlashlightOn = false;

private:
	void InitAbilityActorInfo();
	void GrantDefaultAbilities();
	void ApplyJumpMovementSettings();
	void RegisterJumpMappingContext();
	void UnregisterJumpMappingContext();
	UInputAction* GetJumpInputAction() const;
	void ApplyFlashlightVisual();
};
