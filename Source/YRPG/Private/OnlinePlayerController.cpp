// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlinePlayerController.h"

#include "Announcement.h"
#include "MyPlayer.h"
#include "OnlineGameMode.h"
#include "OnlinePlayerHUD.h"
#include "PlayerOverlap.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void AOnlinePlayerController::SetHUDHealth(float Health,float MaxHealth)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if(OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->HealthBar)
	{
		const float HealthPercent = Health / MaxHealth;
		OnlinePlayerHUD->PlayerOverlap->HealthBar->SetPercent(HealthPercent);
	}
	else
	{
		bInitializePlayerOverlap = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializePlayerOverlap = true;
		HUDStamina = Stamina;
		HUDMaxStamina = MaxStamina;
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
	else
	{
		bInitializePlayerOverlap = true;
		HUDScore = Score;
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
	else
	{
		bInitializePlayerOverlap = true;
		HUDDefeats = Defeats;
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
	ServerCheckMatchState();
	
}

void AOnlinePlayerController::SetHUDTime()
{
	float TimeLeft =0;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncement(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
	
}

void AOnlinePlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AOnlinePlayerController::PollInit()
{
	if(PlayerOverlap == nullptr)
	{
		if(OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap)
		{
			PlayerOverlap = OnlinePlayerHUD->PlayerOverlap;
			if(PlayerOverlap)
			{
				SetHUDHealth(HUDHealth,HUDMaxHealth);
				SetHUDStamina(HUDStamina,HUDMaxStamina);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}


void AOnlinePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                    float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AOnlinePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceipt);
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

void AOnlinePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if(OnlinePlayerHUD && OnlinePlayerHUD->PlayerOverlap && OnlinePlayerHUD->PlayerOverlap->MatchCountdownText)
	{
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		OnlinePlayerHUD->PlayerOverlap->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AOnlinePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
}

float AOnlinePlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	
   	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AOnlinePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AOnlinePlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AOnlinePlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AOnlinePlayerController::HandleMatchHasStarted()
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if(OnlinePlayerHUD)
	{
		if(OnlinePlayerHUD->Announcement)
		{
			OnlinePlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		OnlinePlayerHUD->AddPlayerOverlap();
		
	}
		
}

void AOnlinePlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch,float Warmup,float Match,float StaringTime)
{
	
		WarmupTime = Warmup;
		MatchTime = Match;
		LevelStartingTime = StaringTime;
		MatchState = StateOfMatch;
	    OnMatchStateSet(MatchState);
	if(OnlinePlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		OnlinePlayerHUD->AddAnnouncement();
	}
}

void AOnlinePlayerController::ServerCheckMatchState_Implementation()
{
	AOnlineGameMode* GameMode = Cast<AOnlineGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState,WarmupTime,MatchTime,LevelStartingTime);
		if(OnlinePlayerHUD && MatchState == MatchState::WaitingToStart)
		{
			OnlinePlayerHUD->AddAnnouncement();
		}
	}
}

void AOnlinePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOnlinePlayerController,MatchState);
}

void AOnlinePlayerController::SetHUDAnnouncement(float CountdownTime)
{
	OnlinePlayerHUD = OnlinePlayerHUD == nullptr ? Cast<AOnlinePlayerHUD>(GetHUD()) : OnlinePlayerHUD;
	if(OnlinePlayerHUD && OnlinePlayerHUD->Announcement && OnlinePlayerHUD->Announcement->WarmupTime)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		OnlinePlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}
