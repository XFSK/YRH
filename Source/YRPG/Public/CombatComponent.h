

#pragma once

#include "CoreMinimal.h"
#include "CombatState.h"
#include "Weapon.h"
#include "Components/ActorComponent.h"
#include "OnlinePlayerHUD.h"
#include "Templates/SharedPointer.h"
#include "WeaponType.h"
#include "CombatComponent.generated.h"
#define  TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YRPG_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AMyPlayer;
protected:
	
	virtual void BeginPlay() override;

	void FireButtonPressed(bool bPressed);
public:	
	void EquipWeapon(class AWeapon* WeaponToEquip);

	void PickupWeapon(class AWeapon* WeaponToPickup);

	void HolsterWeapon(class AWeapon* WeaponToHolst);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast,Reliable)
	void ServerPlayMontage(UAnimMontage* AnimMontage);

	UFUNCTION(Server,Reliable)
	void ServerReload();

    void HandleReload();

	int32 AmmoToReload();

	UFUNCTION(BlueprintCallable)
    void OnEquipWeaponFinish();
	
	UFUNCTION(NetMulticast,Reliable)
	void WeaponCreate(AWeapon* PickWeapon);

	UFUNCTION(NetMulticast,Reliable)
	void PlayerFireMontage(UAnimMontage* AnimMontage,UParticleSystem* ParticleSystem,const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(Server,Reliable)
	void ServerPlayFireMontage(UAnimMontage* AnimMontage,UParticleSystem* ParticleSystem,const FVector_NetQuantize& TraceHitTarget);  
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UCameraShakeBase> FireCamShake;

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	FHUDPackage HUDPackage;

	FHitResult HitResult;

	UPROPERTY(Replicated,BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing=OnRep_PickUpWeapon,BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	class AWeapon* PickuppedWeapon;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	
	TMap<EWeaponType,int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 240;
	
	void InitializeCarriedAmmo();

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();
private:

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	
	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming);

    void SetAiming(bool bIsAiming);
	
	UPROPERTY()
	class AMyPlayer* Player;

    class AOnlinePlayerController* Controller;

	class AOnlinePlayerHUD* HUD;
	

	UPROPERTY(Replicated,BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	bool bAiming;

	UFUNCTION()
	void OnRep_PickUpWeapon();
	
	bool bFireButtonPressed;

	void SetHUDCrosshairs(float DeltaTime);

	float CrosshairVelocityFactor;

	float CrosshairInAirFactor;

	float CrosshairAimFactor;

	float CrosshairShootingFatcor;

	FTimerHandle FireTimer;

	

	void StartFireTimer();
	
	void FireTimerFinished();

	void Fire();
	
	
	bool bCanFire = true;

	bool CanFire();

	void UpdateAmmoValue();
};
