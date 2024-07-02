// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPlayer.h"

#include "AIController.h"
#include "HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"

// Sets default values
AEnemyPlayer::AEnemyPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bCombat = false;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComponent->MaxHealth = 100;
	HealthComponent->CurrentHealth = HealthComponent->MaxHealth;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetRootComponent());
	
}

// Called when the game starts or when spawned
void AEnemyPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


FVector AEnemyPlayer::GetPawnViewLocation() const
{
	if(CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void AEnemyPlayer::EnemyFire() 
 {
	if(CurrentWeapon)
	{
		FVector WeaponStartLocation;
		FVector WeaponEndLocation;
		FRotator ActorRotation;
		FHitResult WeaponHit;
		FHitResult HitInfo;
		AActor* HitActor;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(this);
		GetActorEyesViewPoint(WeaponStartLocation,ActorRotation);
		WeaponEndLocation = WeaponStartLocation + ActorRotation.Vector() * 10000;
		if(GetWorld()->LineTraceSingleByChannel(WeaponHit,WeaponStartLocation,WeaponEndLocation,ECC_Visibility,QueryParams))
		{
			DrawDebugLine(GetWorld(),WeaponStartLocation,WeaponEndLocation,FColor::Red,false,5,1,4);
			HitActor = WeaponHit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor,20,WeaponStartLocation,HitInfo,Cast<AAIController>(GetController()),this,DamageType);
		}
	}
}
