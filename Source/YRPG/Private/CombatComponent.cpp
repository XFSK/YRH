#include "CombatComponent.h"

#include "MyPlayer.h"
#include "OnlinePlayerController.h"
#include "OnlinePlayerHUD.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Field/FieldSystemNoiseAlgo.h"

UCombatComponent::UCombatComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
	
}



void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Player->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
	
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bPressed && EquippedWeapon)
	{
		Fire();
	}
	
}

void UCombatComponent::PickupWeapon(AWeapon* WeaponToPickup)
{
	if(Player == nullptr || WeaponToPickup == nullptr) return;
	if(PickuppedWeapon)
	{
		PickuppedWeapon->Dropped();
	}
	PickuppedWeapon = WeaponToPickup;
	PickuppedWeapon->SetOwner(Player);
	PickuppedWeapon->SetHUDAmmo();
	Player->PlayerReloadMontage = PickuppedWeapon->WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("PlayerReloadMontage"))->PlayerReloadMontage;
	if(CarriedAmmoMap.Contains(PickuppedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[PickuppedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AOnlinePlayerController>(Player->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	PickuppedWeapon->SetWeaponState(EWeaponState::EWS_PickUpped);
	PickuppedWeapon->AttachToComponent(Player->GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,Player->RightBackSocketName); 
	Player->LeftHandIKSwitch();
	PickuppedWeapon->DynamicMaterialInstance->SetScalarParameterValue(TEXT("武器生成开关"),1.f);
	PickuppedWeapon->StartWeaponCreate();
}


void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Player == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetOwner(Player);
	UAnimMontage* ChangeMontage;
	ChangeMontage = EquippedWeapon->WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("EquipMontage"))->EquipMontage;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->HasPickUp = true;
	PickuppedWeapon = nullptr;
	if(ChangeMontage)
	{
		ServerPlayMontage(ChangeMontage);
	}
}


void UCombatComponent::HolsterWeapon(AWeapon* WeaponToHolst)
{
	if(Player == nullptr || WeaponToHolst == nullptr) return;
	PickuppedWeapon = WeaponToHolst;
	PickuppedWeapon->SetOwner(Player);
	UAnimMontage* ChangeMontage;
	EquippedWeapon = nullptr;
	ChangeMontage = PickuppedWeapon->WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("HolsterMontage"))->HolsterMontage;
	if(ChangeMontage)
	{
		ServerPlayMontage(ChangeMontage);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UCombatComponent,EquippedWeapon,COND_None);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatComponent, PickuppedWeapon, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UCombatComponent,bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent,CombatState);
/**	 DOREPLIFETIME(UCombatComponent,Player);  **/
}

void UCombatComponent::HandleReload()
{
	
}

int32 UCombatComponent::AmmoToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->MagCapacity - EquippedWeapon->Ammo;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmoCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag,AmmoCarried);
		return FMath::Clamp(RoomInMag,0,Least);
	}
	return 0;
}

void UCombatComponent::OnEquipWeaponFinish()
{
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Player == nullptr || EquippedWeapon == nullptr)  return;
	
	UAnimMontage* AnimMontage = Player->PlayerReloadMontage;
	ServerPlayMontage(AnimMontage);
	HandleReload();
	CombatState = ECombatState::ECS_Reloading;
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading :
		HandleReload();

	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::PlayerFireMontage_Implementation(UAnimMontage* AnimMontage,UParticleSystem* ParticleSystem,const FVector_NetQuantize& TraceHitTarget)
{
	UAnimInstance* AnimInstance = Player->GetMesh()->GetAnimInstance();
	if(AnimInstance && CombatState == ECombatState::ECS_Unoccupied)
	{
		AnimInstance->Montage_Play(AnimMontage);
		if(Player->IsLocallyControlled() && Player->Controller)
		{
			APlayerController* PC = Cast<APlayerController>(Player->GetController());
			PC->ClientStartCameraShake(FireCamShake);
		}
		UGameplayStatics::SpawnEmitterAttached(ParticleSystem,EquippedWeapon->SkeletalMeshComponent,FName("Muzzle"));
		if(EquippedWeapon && Player->HasAuthority())
		{
			EquippedWeapon->Fire(TraceHitTarget);
		}
	}
	
}


void UCombatComponent::ServerPlayFireMontage_Implementation(UAnimMontage* AnimMontage,UParticleSystem* ParticleSystem,const FVector_NetQuantize& TraceHitTarget)
{
	PlayerFireMontage(AnimMontage,ParticleSystem,TraceHitTarget);
	
} 

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f,ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld =  UGameplayStatics::DeprojectScreenToWorld(
		 UGameplayStatics::GetPlayerController(this,0),
		 CrosshairLocation,
		 CrosshairWorldPosition,
		 CrosshairWorldDirection
	);
	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if(Player)
		{
			float DistanceToPlayer = (Player->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToPlayer + 30.f);
			
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		FCollisionQueryParams QueryParams;
		FCollisionResponseParams ResponseParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(EquippedWeapon);
		QueryParams.AddIgnoredActor(Player);
		ResponseParams = FCollisionResponseParams::DefaultResponseParam;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			QueryParams,
			ResponseParams
		);
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}


void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AOnlinePlayerController>(Player->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Add(EWeaponType::EWT_AssaultRifle,StartingARAmmo);
}



void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	
}

void UCombatComponent::OnRep_PickUpWeapon()
{
	if(PickuppedWeapon && Player && !PickuppedWeapon->HasPickUp)
	{
		PickuppedWeapon->SetWeaponState(EWeaponState::EWS_PickUpped);
		PickuppedWeapon->AttachToComponent(Player->GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,Player->RightBackSocketName); 
		PickuppedWeapon->DynamicMaterialInstance->SetScalarParameterValue(TEXT("武器生成开关"),1.f);
		Player->LeftHandIKSwitch();
		if(PickuppedWeapon)
		{
			PickuppedWeapon->StartWeaponCreate();
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Player == nullptr || Player->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AOnlinePlayerController>(Player->Controller) : Controller;
	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<AOnlinePlayerHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
			}
			FVector2D WalkSpeedRange(0.f,Player->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f,1.f);
			FVector Velocity = Player->CurrentPlayerSpeed;
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelocityMultiplierRange,Velocity.Size());

			if(Player->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairVelocityFactor,10.0f,DeltaTime,5);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairVelocityFactor,0.f,DeltaTime,30.f);
			}

			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,0.58f,DeltaTime,30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,0.f,DeltaTime,20.f);
			}
			CrosshairShootingFatcor = FMath::FInterpTo(CrosshairShootingFatcor,0.f,DeltaTime,30.f);
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFatcor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Player == nullptr) return;
	
	Player->GetWorldTimerManager().SetTimer(FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if(EquippedWeapon == nullptr) return;
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		CrosshairShootingFatcor = 3.f;
		if(Player)
		{
			if(EquippedWeapon && EquippedWeapon->MuzzleEffect && Player->PlayerFireMontage)
			{
				ServerPlayFireMontage(Player->PlayerFireMontage,EquippedWeapon->MuzzleEffect,HitResult.ImpactPoint);
				bCanFire = false;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::Reload()
{
	if(!EquippedWeapon  || EquippedWeapon->Ammo == EquippedWeapon->MagCapacity) return;
	if(CarriedAmmo > 0)
	{
		ServerReload();
		CombatState = ECombatState::ECS_Reloading;
	}
}

void UCombatComponent::FinishReload()
{
	if(Player == nullptr) return;
	if(Player->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValue();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}


bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied;
}

void UCombatComponent::UpdateAmmoValue()
{
	if(Player == nullptr || EquippedWeapon == nullptr)  return;
	int32 ReloadAmount = AmmoToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
	Controller = Controller == nullptr ? Cast<AOnlinePlayerController>(Player->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::WeaponCreate_Implementation(AWeapon* PickWeapon)
{
	PickWeapon->StartWeaponCreate();
}

void UCombatComponent::ServerPlayMontage_Implementation(UAnimMontage* AnimMontage)
{
	if(AnimMontage)
	{
		Player->CurrentWeaponAnimMontage = AnimMontage;
		Player->PlayWeaponMontage();
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);
	TraceUnderCrosshairs(HitResult);
}

