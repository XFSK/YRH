// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "InputActionValue.h"
#include "InteractWithCrosshairInterface.h"
#include "Weapon.h"
#include "OnlinePlayerState.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyPlayer.generated.h"

UCLASS()
class YRPG_API AMyPlayer : public ACharacter,public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyPlayer();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float Direction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Input,meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Input,meta=(AllowPrivateAccess = "true"))
	class UInputAction* RunAction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Input,meta=(AllowPrivateAccess = "true"))
	class UInputAction* PickUpAction;

    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess = "true"))
	class UInputAction* Equip_Action;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess= "true"))
	class UInputAction* Holster_Action;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess= "true"))
	class UInputAction* DrawWeapon_Action;
	
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess= "true"))
	class UInputAction* CharacterFire_Action;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterFireHold_Action;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterOpenPackage_Action;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterUseInventory1;
 
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterUseInventory2;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterUseInventory3;

    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input,meta=(AllowPrivateAccess="true"))
	class UInputAction* CharacterReloadAction;
	
	UFUNCTION(BlueprintCallable)
	virtual FVector GetPawnViewLocation() const override;

    UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class USceneComponent* WeaponSceneComponent;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class USpringArmComponent* WeaponArmComponent;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName RightHandSocketName;

	void HideCameraIfPlayerClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	float TurnBaseRate;

	float PitchBaseRate;

	void StartRun();

	void EndRun();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly)
	FString Name;
	
    UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class AWeapon* CurrentWeapon;
	
	UFUNCTION(BlueprintCallable)
	void PickUpWeapon();


	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FName RightBackSocketName;
	
/**
	UFUNCTION(BlueprintCallable)
	void Drawgun();
	**/

	void PollInit();

	UPROPERTY()
	class AOnlinePlayerState* OnlinePlayerState;
	
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_Elim();

    void Elim();
	
	UFUNCTION(BlueprintCallable)
	void EquipmentWeapons();

	UFUNCTION(BlueprintImplementableEvent)
	void LeftHandIKSwitch();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayWeaponMontage();
	
	UPROPERTY(Replicated,EditAnywhere,BlueprintReadOnly)
	class UAnimMontage* CurrentWeaponAnimMontage;

	UFUNCTION(BlueprintCallable)
	void HolsterWeapons();

	FTimerHandle ElimTimer;

	void ElimTimerFinished();

	FOnTimelineFloat DissolveTrack;

    FOnTimelineFloat GlowTrack;
	
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeLine;

    UPROPERTY(VisibleAnywhere)
	UTimelineComponent* GlowTimeLine;
	
	UPROPERTY(EditDefaultsOnly)
    float ElimDelay = 3.f;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UFUNCTION()
    void UpdateGlowMaterial(float GlowValue);
	
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

    UPROPERTY(EditAnywhere)
	UCurveFloat* GlowCurve;
	
	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance3;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance4;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance5;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance6;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance7;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance8;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance9;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance1;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance2;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance3;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance4;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance5;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance6;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance7;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance8;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DissolveMaterialInstance9;
	/**
	UFUNCTION(BlueprintImplementableEvent)
	void Draw_Gun();

	**/
	
/**
	UFUNCTION(BlueprintCallable)
	void FireStart();

	UFUNCTION(BlueprintCallable)
	void FireEnd();

**/
    void FireButtonPressed();

	void FireButtonReleased();

    void ReloadButtonPressed();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageType;

	UFUNCTION()
	void FireHoldStart();

	UFUNCTION()
	void FireHoldEnd();
	
	UPROPERTY(EditAnywhere)
	class UAnimMontage* HitReactMontage;

    UPROPERTY(EditAnywhere)
	class UAnimMontage* ElimMontage;

	
	
	
	void PlayHitReactMontage();

	UFUNCTION()
    void PlayElimMontage();

	bool bElimmed;
	
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastHit();
	
	float StartView;

	void UpdateHUDHealth();

	void UpdateHUDStamina();
	
	float CurrentView;

	float InterpSpeed;

	UPROPERTY(Replicated,BlueprintReadOnly)
	bool GunHold;

    UFUNCTION(Server,Reliable)
	void ServerSetUseControllerRotation(bool bIsAim);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OpenPackage();

	UFUNCTION(BlueprintImplementableEvent)
	void UseInventoryOne();

	UFUNCTION(BlueprintImplementableEvent)
	void UseInventoryTwo();

	UFUNCTION(BlueprintImplementableEvent)
	void UseInventoryThree();

	UPROPERTY(Replicated,BlueprintReadOnly)
	bool IsRunning;

	UPROPERTY(Replicated,BlueprintReadOnly)
	bool bIsCombat;

	//BlueprintCallable Function To Set And Join The Session
	UFUNCTION(BlueprintCallable)
	void OpenTest();

	UFUNCTION(BlueprintCallable)
	void CallOpenLevel(const FString& Address);

	UFUNCTION(BlueprintCallable)
	void CallClientTravel(const FString& Address);
	
	 TSharedPtr<class IOnlineSession,ESPMode::ThreadSafe> OnlineSessionInterface;

     virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetOverlappingWeapon(AWeapon* Weapon);
	
	UPROPERTY(Replicated,BlueprintReadOnly)
	bool bIsJump;

    UFUNCTION(NetMulticast,Reliable)
	void ServerJump(bool Jumping);
	
	virtual void Jump() override;
	
	virtual void StopJumping() override;

	UFUNCTION(BlueprintCallable)
	void SetInuptMode(bool bUIShow);

	UFUNCTION()
    bool IsAiming();
	
	UPROPERTY(Replicated,BlueprintReadOnly)
	float AimPitchOffset;

	void SetAimOffset();

	UFUNCTION(Server,Reliable)
	void ServerSetAimOffset();

	UPROPERTY(Replicated,EditAnywhere)
	class UAnimMontage* PlayerFireMontage;
	
	UPROPERTY(Replicated,EditAnywhere)
	class UAnimMontage* PlayerReloadMontage;
	
	UPROPERTY(BlueprintReadOnly,Replicated,meta=(AllowPrivateAccess = "true"))
	FVector CurrentPlayerSpeed;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor,float Damaged,const UDamageType* DamagedType,class AController* InstigatorController,AActor* DamageCauser);

    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth;}

	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina;}
	FORCEINLINE float GetMaxStamina() const { return MaxStamina;}

	ECombatState GetCombatState() const;
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	
private:
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	
	virtual void PostInitializeComponents() override;
	
	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server,Reliable)
	void ServerPickupButtonPressed();

	UFUNCTION(Server,Reliable)
	void ServerHolsterButtonPressed();

	UFUNCTION(Server,Reliable)
    void Server_Run(bool bIsRun);

	UPROPERTY(EditAnywhere)
	float WeaponZoomSize;

	bool bRotateRootBone;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100;

	UPROPERTY(ReplicatedUsing = OnRep_Health,VisibleAnywhere)
	float CurrentHealth = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(EditAnywhere)
	float MaxStamina = 200;

	float StaminaDecreaseRate = 5.f;
	
	UPROPERTY(ReplicatedUsing=OnRep_Stamina,VisibleAnywhere)
	float CurrentStamina = 200;

	UFUNCTION()
	void OnRep_Stamina();

	class AOnlinePlayerController* OnlinePlayerController;

	
};
