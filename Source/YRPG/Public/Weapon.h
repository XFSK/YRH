// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PackageObject.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_PickUpped UMETA(DisplayName = "PickUpped"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};



USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	class UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* HolsterMontage;

	UPROPERTY(EditAnywhere)
	float Damage;

	UPROPERTY(EditAnywhere)
	class UMaterialInstance* WeaponMaterial;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* PlayerFireMontage;

    UPROPERTY(EditAnywhere)
	class USoundCue* WeaponFieSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* WeaponFireParticle;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* PlayerReloadMontage;

	UPROPERTY(EditAnywhere)
	class USoundCue* WeaponCreateSound;
};


UCLASS()
class YRPG_API AWeapon : public APackageObject
{
	GENERATED_BODY()

public:
     AWeapon();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class USkeletalMeshComponent* SkeletalMeshComponent;
	

	UPROPERTY(Replicated,EditDefaultsOnly,BlueprintReadOnly,Category= Animation)
	class UDataTable* WeaponDataTable;

	UFUNCTION()
	void Fire(const FVector& HitTarget);


private:
	UPROPERTY(VisibleAnywhere,ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

    UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* PickUpText;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,meta=(AllowPrivateAccess = "true"))
    class USphereComponent* AreaSphere;

	UFUNCTION()
	void OnRep_WeaponState();

	

	UFUNCTION()
    void OnRep_Ammo();
	
    void SpendRound();
	
	UPROPERTY(EditAnywhere)
	class USoundCue* WeaponCreateSound;

	UPROPERTY(EditAnywhere)
    EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle;
	
public:
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

    void AddAmmo(int32 AmmoToAdd);
	
	UPROPERTY(EditAnywhere,ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;
	
    UPROPERTY(VisibleAnywhere)
	class UTimelineComponent* WeaponCreateTimelineComponent;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* WeaponCreateCurve;

	FOnTimelineFloat WeaponCreateTrack;
	
	UFUNCTION()
	void RendersStarted(float Value);

	void StartWeaponCreate();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,ReplicatedUsing= On_MaterialChanged)
	class UMaterialInterface* BaseMaterial;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class UMaterialInstanceDynamic* DynamicMaterialInstance;

    UPROPERTY(Replicated,EditAnywhere)
	bool HasPickUp;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingObjectClass;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult); //

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void ShowPickUpWidget(bool bShowWidget);

	void SetWeaponState(EWeaponState State);
    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere;}
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType;}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
    UFUNCTION()
	void On_MaterialChanged();
	 
	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(Replicated,EditAnywhere)
	class UParticleSystem* MuzzleEffect;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	float FireDelay = .15f;

	void Dropped();

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();

	bool IsEmpty();
};
