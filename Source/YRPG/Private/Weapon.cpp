// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Casing.h"
#include "CombatComponent.h"
#include "MyPlayer.h"
#include "OnlinePlayerController.h"
#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DSP/AudioDebuggingUtilities.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SetRootComponent(SkeletalMeshComponent);
	bReplicates = true;
	SkeletalMeshComponent->SetIsReplicated(true);
	if(GetLocalRole() == ROLE_Authority)
	{
		AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
		AreaSphere->SetupAttachment(RootComponent);
		AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	}
	PickUpText = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpText"));
	PickUpText->SetupAttachment(RootComponent);
	PickUpText->SetVisibility(false);
	PickUpText->MarkRenderStateDirty();
	WeaponCreateTimelineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("WeaponCreateTimelineComponent"));
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	PickUpText->SetVisibility(false);
	WeaponCreateTrack.BindDynamic(this,&AWeapon::RendersStarted);
	WeaponCreateTimelineComponent->AddInterpFloat(WeaponCreateCurve,WeaponCreateTrack);
	WeaponCreateSound = WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("WeaponCreateSound"))->WeaponCreateSound;
    WeaponCreateSound->PitchMultiplier = 0.5f;
	if(HasAuthority())
	{
		BaseMaterial = WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("WeaponMaterialInstance"))->WeaponMaterial;
		MuzzleEffect = WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("FireMuzzle"))->WeaponFireParticle;
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn,ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this,&AWeapon::OnSphereOverlapEnd);
		if(BaseMaterial)
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial,this);
			if(DynamicMaterialInstance)
			{
				DynamicMaterialInstance->SetScalarParameterValue(TEXT("武器生成开关"),0.f);
			}
			SkeletalMeshComponent->SetMaterial(0,DynamicMaterialInstance);
		}
	}
	SkeletalMeshComponent->SetSimulatePhysics(true);
	SkeletalMeshComponent->SetEnableGravity(true);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
}


void AWeapon::Fire(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleSocket = SkeletalMeshComponent->GetSocketByName(FName("Muzzle"));
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if(MuzzleSocket)
	{
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(SkeletalMeshComponent);
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator TargetRotation = ToTarget.Rotation();
	if(ProjectileClass && InstigatorPawn)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;
		UWorld* World = GetWorld();
		if(World)
		{
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParameters
				);
		}
	}
		const USkeletalMeshSocket* AmmoEjectSocket = SkeletalMeshComponent->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			if(CasingObjectClass)
			{
				FTransform CasingSocket = AmmoEjectSocket->GetSocketTransform(SkeletalMeshComponent);
				UWorld* World = GetWorld();
				if(World)
				{
					World->SpawnActor<ACasing>(
						CasingObjectClass,
						CasingSocket.GetLocation(),
						CasingSocket.GetRotation().Rotator()
						);
				}
			}
		
		}
		SpendRound();
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_PickUpped :
		ShowPickUpWidget(false);
		SkeletalMeshComponent->SetSimulatePhysics(false);
		SkeletalMeshComponent->SetEnableGravity(false);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
		
	case EWeaponState::EWS_Dropped:
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMeshComponent->SetSimulatePhysics(true);
		SkeletalMeshComponent->SetEnableGravity(true);
		break;
	}
}


void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1,0,MagCapacity);
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd,0,MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	if(Owner)
	{
		SetHUDAmmo();
	}
}


void AWeapon::RendersStarted(float Value)
{
	if(DynamicMaterialInstance && WeaponCreateTimelineComponent && WeaponCreateCurve)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("生成遮罩"),Value);

		if(Value ==1.f)
		{
			DynamicMaterialInstance->SetScalarParameterValue(TEXT("武器生成开关"),0.f);
		}
	}
	
}


void AWeapon::StartWeaponCreate()
{
	WeaponCreateTimelineComponent->Play();
	UGameplayStatics::PlaySound2D(this,WeaponCreateSound);
}


void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
  AMyPlayer* Player = Cast<AMyPlayer>(OtherActor);
	if(Player)
	{
		Player->SetOverlappingWeapon(this);
	}
}


void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AWeapon::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyPlayer* Player = Cast<AMyPlayer>(OtherActor);
	if(Player)
	{
		Player->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpText)
	{
		PickUpText->SetVisibility(bShowWidget);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_PickUpped :
		ShowPickUpWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMeshComponent->SetSimulatePhysics(false);
		SkeletalMeshComponent->SetEnableGravity(false);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMeshComponent->SetSimulatePhysics(true);
		SkeletalMeshComponent->SetEnableGravity(true);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon,WeaponState);
	DOREPLIFETIME(AWeapon,WeaponDataTable);
	DOREPLIFETIME(AWeapon,HasPickUp);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon,BaseMaterial,COND_None,REPNOTIFY_Always); 
	DOREPLIFETIME(AWeapon,MuzzleEffect);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon,Ammo,COND_None,REPNOTIFY_Always);
}

void AWeapon::On_MaterialChanged()
{
	if(BaseMaterial)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial,this);
		
		if(DynamicMaterialInstance)
		{
			DynamicMaterialInstance->SetScalarParameterValue(TEXT("武器生成开关"),0.f);
		}
		SkeletalMeshComponent->SetMaterial(0,DynamicMaterialInstance);
	}
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld,true);
	SkeletalMeshComponent->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	HasPickUp = false;
	
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	AMyPlayer* OwnerPlayer = Cast<AMyPlayer>(GetOwner());
	if(OwnerPlayer)
	{
		AOnlinePlayerController* OnlinePlayerOwnerController = Cast<AOnlinePlayerController>(OwnerPlayer->Controller);
		if(OnlinePlayerOwnerController)
		{
			OnlinePlayerOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}


/**
void AWeapon::ServerCreateMaterialInstance_Implementation()
{
	CreateMaterialInstance();
	GEngine->AddOnScreenDebugMessage(
		4,
		15.f,
		FColor::Black,
		FString::Printf(TEXT("Hello!")));
		}
		**/

