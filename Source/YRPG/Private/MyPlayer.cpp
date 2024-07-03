// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayer.h"


#include "CombatComponent.h"
#include "EnemyPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthComponent.h"
#include "KismetAnimationLibrary.h"
#include "OnlineGameMode.h"
#include "OnlinePlayerController.h"
#include "OnlineSubsystem.h"
#include "Weapon.h"
#include "AnimNodes/AnimNode_RandomPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "YRPG/YRPG.h"
// Sets default values
AMyPlayer::AMyPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bIsJump = false;
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 300;
	bIsCombat = false;
	bReplicates = true;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch =false;
	bUseControllerRotationRoll = false;
	SpringArmComponent->bUsePawnControlRotation = true;
	WeaponSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerWeaponScence"));
	WeaponArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("WeaponArm"));
	WeaponArmComponent->SetupAttachment(GetRootComponent());
	WeaponSceneComponent->SetupAttachment(WeaponArmComponent);
	WeaponArmComponent->TargetArmLength = 60;
	WeaponArmComponent->bEnableCameraLag = true;
	WeaponArmComponent->bEnableCameraRotationLag = true;
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);
	IsRunning = false;
	bElimmed = false;
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

		/**
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
              -1,
              15.f,
              FColor::Blue,
              FString::Printf(TEXT("Found subsystem %s"),*OnlineSubsystem->GetSubsystemName().ToString())
			);
		}
		**/
	}
  
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);
	NetUpdateFrequency = 66;
	MinNetUpdateFrequency = 33;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	DissolveTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));
	GlowTimeLine = CreateDefaultSubobject<UTimelineComponent>(TEXT("GlowTimeLineComponent"));
}

void AMyPlayer::HideCameraIfPlayerClose()
{
	if(!IsLocallyControlled()) return;
	if((CameraComponent->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && bIsCombat && Combat->EquippedWeapon->SkeletalMeshComponent)
		{
			Combat->EquippedWeapon->SkeletalMeshComponent->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && bIsCombat && Combat->EquippedWeapon->SkeletalMeshComponent)
		{
			Combat->EquippedWeapon->SkeletalMeshComponent->bOwnerNoSee = false;
		}
	}
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	bReplicates = true;
	GetCharacterMovement()->SetIsReplicated(true);
	Super::BeginPlay();
	TurnBaseRate = 65;
	PitchBaseRate = 65;
	GetCharacterMovement()->MaxWalkSpeed = 200;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0,750,0);
	GetCharacterMovement()->JumpZVelocity = 650;
	GetCharacterMovement()->AirControl = 0.2f;
	if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext,0);
		}
	}
	StartView = CameraComponent->FieldOfView;
	CurrentView = StartView;
	InterpSpeed = 15;
	GunHold = false;
	UpdateHUDStamina();
	UpdateHUDHealth();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&AMyPlayer::ReceiveDamage);
	}
	
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
  if(HasAuthority())
  {
  	CurrentPlayerSpeed = GetVelocity();
  	SetAimOffset();
  	if(IsRunning && CurrentStamina!=0)
  	{
  		CurrentStamina = CurrentStamina - (StaminaDecreaseRate * DeltaTime);
  	}
  	else
  	{
  		CurrentStamina = FMath::Clamp(CurrentStamina +(StaminaDecreaseRate * DeltaTime),0.f,MaxStamina);
  	}
  	UpdateHUDStamina();
  }
  else
  {
	  ServerSetAimOffset();
  }
	Direction = UKismetAnimationLibrary::CalculateDirection(GetVelocity(),GetActorRotation());
	if(CameraComponent)
	{
		float TransitionView;
		TransitionView = FMath::FInterpTo(CameraComponent->FieldOfView,CurrentView,DeltaTime,InterpSpeed);
		CameraComponent->SetFieldOfView(TransitionView);
	}
		HideCameraIfPlayerClose();

	PollInit();
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPlayer::Move);

		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&AMyPlayer::Look);

		EnhancedInputComponent->BindAction(RunAction,ETriggerEvent::Triggered,this,&AMyPlayer::StartRun);

		EnhancedInputComponent->BindAction(RunAction,ETriggerEvent::Completed,this,&AMyPlayer::EndRun);

		EnhancedInputComponent->BindAction(Equip_Action,ETriggerEvent::Completed,this,&AMyPlayer::EquipmentWeapons);

		
		EnhancedInputComponent->BindAction(PickUpAction,ETriggerEvent::Triggered,this,&AMyPlayer::PickUpWeapon);
		
		EnhancedInputComponent->BindAction(Holster_Action,ETriggerEvent::Ongoing,this,&AMyPlayer::HolsterWeapons); 
		/**
		EnhancedInputComponent->BindAction(DrawWeapon_Action,ETriggerEvent::Triggered,this,&AMyPlayer::Drawgun);
	**/
		EnhancedInputComponent->BindAction(CharacterFire_Action,ETriggerEvent::Started,this,&AMyPlayer::FireButtonPressed);
	
		EnhancedInputComponent->BindAction(CharacterFire_Action,ETriggerEvent::Completed,this,&AMyPlayer::FireButtonReleased);
         
		EnhancedInputComponent->BindAction(CharacterFireHold_Action,ETriggerEvent::Started,this,&AMyPlayer::FireHoldStart);

		EnhancedInputComponent->BindAction(CharacterFireHold_Action,ETriggerEvent::Completed,this,&AMyPlayer::FireHoldEnd);

		EnhancedInputComponent->BindAction(CharacterOpenPackage_Action,ETriggerEvent::Completed,this,&AMyPlayer::OpenPackage);

		EnhancedInputComponent->BindAction(CharacterUseInventory1,ETriggerEvent::Completed,this,&AMyPlayer::UseInventoryOne);

		EnhancedInputComponent->BindAction(CharacterUseInventory2,ETriggerEvent::Completed,this,&AMyPlayer::UseInventoryTwo);

		EnhancedInputComponent->BindAction(CharacterUseInventory3,ETriggerEvent::Completed,this,&AMyPlayer::UseInventoryThree);

		EnhancedInputComponent->BindAction(CharacterReloadAction,ETriggerEvent::Started,this,&AMyPlayer::ReloadButtonPressed);
	}

}

void AMyPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2d>();
	if(Controller)
	{
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection,MovementVector.Y);
		AddMovementInput(RightDirection,MovementVector.X);
	}
	
}


void AMyPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if(Controller)
	{
		AddControllerPitchInput(LookVector.Y);
		AddControllerYawInput(LookVector.X);
	}
}


void AMyPlayer::StartRun()
{
	if(GunHold)
	{
		FireHoldEnd();
	}
	if(HasAuthority())
	{
		
		if(CurrentStamina >=5)
		{
			GetCharacterMovement()->MaxWalkSpeed = 600;
			IsRunning = true;
			if(GunHold)
			{
				CameraComponent->SetFieldOfView(StartView);
				bUseControllerRotationYaw = false;
			}
		
		}
		if(CurrentStamina <=0)
		{
			EndRun();
		}
	}
	else
	{
		Server_Run(true);
	}
}

void AMyPlayer::EndRun()
{
	if(HasAuthority())
	{
		IsRunning = false;
		if(bIsCombat)
		{
			GetCharacterMovement()->MaxWalkSpeed = 480;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 200;
		}
		if(GunHold)
		{
			bUseControllerRotationYaw = true;
		}
	}
	else
	{
		Server_Run(false);
	}
}

FVector AMyPlayer::GetPawnViewLocation() const
{
	if(CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

/**
void AMyPlayer::Drawgun()
{
	if(CurrentWeapon)
	{
		Draw_Gun();
		bCombatReady = false;
		CurrentWeapon = nullptr;
	}
}
**/

void AMyPlayer::PickUpWeapon()
{
	if(Combat && Combat->EquippedWeapon ==nullptr)
	{
		if(HasAuthority())
		{
			Combat->PickupWeapon(OverlappingWeapon);
		}
		else
		{
			ServerPickupButtonPressed();
		}
		OverlappingWeapon = nullptr;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
				5,
				15.f,
				FColor::Black,
				FString::Printf(TEXT("Combat is null!")));
	}
}


//在角色生成的时候检查任何还未创建的类，主要包括HUD
void AMyPlayer::PollInit()
{
	if(OnlinePlayerState == nullptr)
	{
		OnlinePlayerState = GetPlayerState<AOnlinePlayerState>();
		if(OnlinePlayerState)
		{
			OnlinePlayerState->AddToScore(0.f);
			OnlinePlayerState->AddToDefeats(0);
		}
	}
}

void AMyPlayer::Multicast_Elim_Implementation()
{
	if(OnlinePlayerController)
	{
		OnlinePlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	if(DissolveMaterialInstance1 && DissolveMaterialInstance2 && DissolveMaterialInstance3 && DissolveMaterialInstance4 && DissolveMaterialInstance5 && DissolveMaterialInstance6 && DissolveMaterialInstance7 && DissolveMaterialInstance8 && DissolveMaterialInstance9)
	{
		DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1,this);
		DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance2,this);
		DynamicDissolveMaterialInstance3 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance3,this);
		DynamicDissolveMaterialInstance4 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance4,this);
		DynamicDissolveMaterialInstance5 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance5,this);
		DynamicDissolveMaterialInstance6 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance6,this);
		DynamicDissolveMaterialInstance7 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance7,this);
		DynamicDissolveMaterialInstance8 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance8,this);
		DynamicDissolveMaterialInstance9 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance9,this);
		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance1);
		GetMesh()->SetMaterial(1,DynamicDissolveMaterialInstance2);
		GetMesh()->SetMaterial(2,DynamicDissolveMaterialInstance3);
		GetMesh()->SetMaterial(3,DynamicDissolveMaterialInstance4);
		GetMesh()->SetMaterial(4,DynamicDissolveMaterialInstance5);
		GetMesh()->SetMaterial(5,DynamicDissolveMaterialInstance6);
		GetMesh()->SetMaterial(6,DynamicDissolveMaterialInstance7);
		GetMesh()->SetMaterial(7,DynamicDissolveMaterialInstance8);
		GetMesh()->SetMaterial(8,DynamicDissolveMaterialInstance9);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance7->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance7->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance8->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance8->SetScalarParameterValue(TEXT("Glow"),0.f);
		DynamicDissolveMaterialInstance9->SetScalarParameterValue(TEXT("Dissolve"),-0.6f);
		DynamicDissolveMaterialInstance9->SetScalarParameterValue(TEXT("Glow"),0.f);
	}
	StartDissolve();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(OnlinePlayerController)
	{
		DisableInput(OnlinePlayerController);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMyPlayer::Elim()
{
	if(Combat && (Combat->PickuppedWeapon || Combat->EquippedWeapon))
	{
		if(Combat->PickuppedWeapon)
		{
			Combat->PickuppedWeapon->Dropped();
		}
		else if(Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->Dropped();
		}
	}
	Multicast_Elim();
	GetWorldTimerManager().SetTimer(ElimTimer,this,&AMyPlayer::ElimTimerFinished,ElimDelay);
}

void AMyPlayer::EquipmentWeapons()
{
	if(Combat && Combat->PickuppedWeapon)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(Combat->PickuppedWeapon);
			GetCharacterMovement()->MaxWalkSpeed = 480;
			bIsCombat = true;
			PlayerFireMontage = Combat->EquippedWeapon->WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("PlayerFireMontage"))->PlayerFireMontage;
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}


void AMyPlayer::HolsterWeapons()
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->HolsterWeapon(Combat->EquippedWeapon);
			GetCharacterMovement()->MaxWalkSpeed = 200;
			bIsCombat = false;
		}
		else
		{
			ServerHolsterButtonPressed();
		}
	}
}

void AMyPlayer::ElimTimerFinished()
{
	AOnlineGameMode* OnlineGameMode = GetWorld()->GetAuthGameMode<AOnlineGameMode>();
	if(OnlineGameMode)
	{
		OnlineGameMode->RequestRespawn(this,Controller);
	}
}

void AMyPlayer::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance1 && DynamicDissolveMaterialInstance2 && DynamicDissolveMaterialInstance3 && DynamicDissolveMaterialInstance4 && DynamicDissolveMaterialInstance5 && DynamicDissolveMaterialInstance6 && DynamicDissolveMaterialInstance7 && DynamicDissolveMaterialInstance8 && DynamicDissolveMaterialInstance9)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance7->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance8->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
		DynamicDissolveMaterialInstance9->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
	}
}

void AMyPlayer::UpdateGlowMaterial(float GlowValue)
{
	if(DynamicDissolveMaterialInstance1 && DynamicDissolveMaterialInstance2 && DynamicDissolveMaterialInstance3 && DynamicDissolveMaterialInstance4 && DynamicDissolveMaterialInstance5 && DynamicDissolveMaterialInstance6 && DynamicDissolveMaterialInstance7 && DynamicDissolveMaterialInstance8 && DynamicDissolveMaterialInstance9)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance7->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance8->SetScalarParameterValue(TEXT("Glow"),GlowValue);
		DynamicDissolveMaterialInstance9->SetScalarParameterValue(TEXT("Glow"),GlowValue);
	}
	
}

void AMyPlayer::StartDissolve()
{
	DissolveTrack.BindDynamic(this,&AMyPlayer::UpdateDissolveMaterial);
	GlowTrack.BindDynamic(this,&AMyPlayer::UpdateGlowMaterial);
	if(DissolveCurve && DissolveTimeLine && GlowTimeLine && GlowCurve)
	{
		DissolveTimeLine->AddInterpFloat(DissolveCurve,DissolveTrack);
		GlowTimeLine->AddInterpFloat(GlowCurve,GlowTrack);
		DissolveTimeLine->Play();
		GlowTimeLine->Play();
	}
}

void AMyPlayer::FireButtonPressed()
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMyPlayer::FireButtonReleased()
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMyPlayer::ReloadButtonPressed()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->Reload();
	}
}


void AMyPlayer::FireHoldStart()
{
	if(Combat && !IsRunning &&bIsCombat)
	{
		if(HasAuthority())
		{
			GunHold = true;
			bUseControllerRotationYaw = true;
			if(CameraComponent)
			{
				CurrentView = WeaponZoomSize;
			}
		}
		else
		{
			if(CameraComponent)
			{
				CurrentView = WeaponZoomSize;
			}
			ServerSetUseControllerRotation(true);
			bUseControllerRotationYaw = true;
		}
		Combat->SetAiming(true);
	}
	
}

void AMyPlayer::FireHoldEnd()
{
	if(Combat && !IsRunning)
	{
		if(HasAuthority())
		{
			GunHold = false;
			bUseControllerRotationYaw = false;
			if(CameraComponent)
			{
				CurrentView = StartView;
			}
		}
		else
		{
			if(CameraComponent)
			{
				CurrentView = StartView;
			}
			ServerSetUseControllerRotation(false);
			bUseControllerRotationYaw = false;
			
		}
		Combat->SetAiming(false);
	}
}

void AMyPlayer::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyPlayer::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}


void AMyPlayer::UpdateHUDHealth()
{
	OnlinePlayerController = OnlinePlayerController == nullptr ?  Cast<AOnlinePlayerController>(GetController()) :OnlinePlayerController ;
	if(OnlinePlayerController)
	{
		OnlinePlayerController->SetHUDHealth(CurrentHealth,MaxHealth);
	}
}

void AMyPlayer::UpdateHUDStamina()
{
	OnlinePlayerController = OnlinePlayerController == nullptr ?  Cast<AOnlinePlayerController>(GetController()) :OnlinePlayerController;
	if(OnlinePlayerController)
	{
		OnlinePlayerController->SetHUDStamina(CurrentStamina,MaxStamina);
	}
}

void AMyPlayer::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

void AMyPlayer::ServerSetUseControllerRotation_Implementation(bool bIsAim)
{
		bUseControllerRotationYaw = bIsAim;
	    GunHold = bIsAim;
}


void AMyPlayer::OpenTest()
{
	UWorld* World = GetWorld();
	if(World)
	{
		World->ServerTravel("/Game/ThirdPerson/Maps/Test?listen'");
	}
}

void AMyPlayer::CallOpenLevel(const FString& Address)
{
	UGameplayStatics::OpenLevel(this,*Address);
}

void AMyPlayer::CallClientTravel(const FString& Address)
{
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if(PlayerController)
	{
		PlayerController->ClientTravel(Address,TRAVEL_Absolute);
	}
}

void AMyPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMyPlayer,OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AMyPlayer,CurrentWeaponAnimMontage);
	DOREPLIFETIME_CONDITION(AMyPlayer,GunHold,COND_None);
	DOREPLIFETIME(AMyPlayer,IsRunning);
	DOREPLIFETIME_CONDITION(AMyPlayer,CurrentPlayerSpeed,COND_None);
	DOREPLIFETIME(AMyPlayer,bIsCombat);
	DOREPLIFETIME(AMyPlayer,AimPitchOffset);
	DOREPLIFETIME(AMyPlayer,PlayerFireMontage);
	DOREPLIFETIME(AMyPlayer,PlayerReloadMontage);
	DOREPLIFETIME(AMyPlayer,CurrentHealth);
	DOREPLIFETIME(AMyPlayer,CurrentStamina);
}

void AMyPlayer::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
	
}


void AMyPlayer::ServerJump_Implementation(bool Jumping)
{
	if(Jumping)
	{
		bIsJump = true;
	}
	else
	{
		bIsJump = false;
	}
}

void AMyPlayer::Jump()
{
	Super::Jump();
	ServerJump(true);
	
}

void AMyPlayer::StopJumping()
{
	Super::StopJumping();
	ServerJump(false);
}

void AMyPlayer::SetInuptMode(bool bUIShow)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if(PC && PC->IsLocalController())
	{
			if(bUIShow)
			{
				FInputModeUIOnly UIOnly;
				UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(UIOnly);
				PC->SetShowMouseCursor(bUIShow);
			}
			else
			{
				FInputModeGameOnly GameOnly;
				PC->SetInputMode(GameOnly);
				PC->SetShowMouseCursor(false);
			}
		
	}
}

bool AMyPlayer::IsAiming()
{
	return (Combat && Combat->bAiming);
}


void AMyPlayer::ReceiveDamage(AActor* DamagedActor, float Damaged, const UDamageType* DamagedType,
                              AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damaged,0.f,MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
	if(CurrentHealth == 0.f)
	{
		AOnlineGameMode* OnlineGameMode = GetWorld()->GetAuthGameMode<AOnlineGameMode>();
		if(OnlineGameMode)
		{
			OnlinePlayerController = OnlinePlayerController == nullptr ? Cast<AOnlinePlayerController>(GetController()) : OnlinePlayerController;
			AOnlinePlayerController* AttackController = Cast<AOnlinePlayerController>(InstigatorController);
		  OnlineGameMode->PlayerEliminated(this,OnlinePlayerController,AttackController);
		}
	}
}

ECombatState AMyPlayer::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void AMyPlayer::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}

void AMyPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Player = this;
	}
}

void AMyPlayer::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
	if(CurrentHealth ==0)
	{
		PlayElimMontage();
	}
}

void AMyPlayer::OnRep_Stamina()
{
	UpdateHUDStamina();
}

void AMyPlayer::SetAimOffset()
{
	FRotator CurrentControllerOffset = GetBaseAimRotation();
	FRotator CurrentActorOffset = GetActorRotation();
	AimPitchOffset = UKismetMathLibrary::NormalizedDeltaRotator(CurrentControllerOffset,CurrentActorOffset).Pitch;
}


void AMyPlayer::ServerSetAimOffset_Implementation()
{
	SetAimOffset();
}


void AMyPlayer::Server_Run_Implementation(bool bIsRun)
{
	if(bIsRun)
	{
		
		if(CurrentStamina >=5)
		{
			IsRunning = true;
			GetCharacterMovement()->MaxWalkSpeed = 600;
			if(GunHold)
			{
				CameraComponent->SetFieldOfView(StartView);
				bUseControllerRotationYaw = false;
			}
		
		}
		if(CurrentStamina <=0)
		{
			Server_Run(false);
		}
	}
	else
	{
		IsRunning = false;
		if(bIsCombat)
		{
			GetCharacterMovement()->MaxWalkSpeed = 480;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 200;
		}
		if(GunHold)
		{
			bUseControllerRotationYaw = true;
		}
	}
}

void AMyPlayer::ServerHolsterButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->HolsterWeapon(Combat->EquippedWeapon);
		bIsCombat = false;
		GetCharacterMovement()->MaxWalkSpeed = 200;
	}
}

void AMyPlayer::ServerPickupButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->PickupWeapon(OverlappingWeapon);
	}
}

void AMyPlayer::ServerEquipButtonPressed_Implementation()
{
	if(Combat && Combat->PickuppedWeapon)
	{
		Combat->EquipWeapon(Combat->PickuppedWeapon);
		GetCharacterMovement()->MaxWalkSpeed = 480;
		bIsCombat = true;
		PlayerFireMontage = Combat->EquippedWeapon->WeaponDataTable->FindRow<FWeaponData>("Rifle",TEXT("PlayerFireMontage"))->PlayerFireMontage;
	}
}


