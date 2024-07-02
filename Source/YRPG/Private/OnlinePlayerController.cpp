// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlinePlayerController.h"

#include "MyPlayer.h"
#include "OnlinePlayerHUD.h"
#include "PlayerOverlap.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void AOnlinePlayerController::SetHUDHealth(float Health,float MaxHealth)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if(OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->HealthBar)
	{
		const float HealthPercent = Health / MaxHealth;
		OnlinePlayerHUD->PlayerOverlap->HealthBar->SetPercent(HealthPercent);
	}
}

void AOnlinePlayerController::SetHUDStamina(float Stamina, float MaxStamina)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;

	if(OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->StaminaBar)
	{
		
		const float StaminaPercent = Stamina / MaxStamina;
		OnlinePlayerHUD->PlayerOverlap->StaminaBar->SetPercent(StaminaPercent);
	}
}

void AOnlinePlayerController::SetHUDScore(float Score)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if (OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		OnlinePlayerHUD->PlayerOverlap->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void AOnlinePlayerController::SetHUDDefeats(int32 Defeats)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if (OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);
		OnlinePlayerHUD->PlayerOverlap->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void AOnlinePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if (OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		OnlinePlayerHUD->PlayerOverlap->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AOnlinePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AMyPlayer* OnlinePlayer = Cast<AMyPlayer>(InPawn);
	if(OnlinePlayer)
	{
		SetHUDHealth(OnlinePlayer->GetCurrentHealth(),OnlinePlayer->GetMaxHealth());
		SetHUDStamina(OnlinePlayer->GetCurrentStamina(),OnlinePlayer->GetMaxStamina());
	}
}

void AOnlinePlayerController::BeginPlay()
{
	Super::BeginPlay();
	OnlinePlayerHUD = Cast<AOnlinePlayerHUD>(GetHUD());
}

void AOnlinePlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if (OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->CarriedAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"),Ammo);
		OnlinePlayerHUD->PlayerOverlap->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}