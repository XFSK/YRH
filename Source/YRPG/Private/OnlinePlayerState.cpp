// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlinePlayerState.h"

#include "MyPlayer.h"
#include "OnlinePlayerController.h"
#include "Net/UnrealNetwork.h"

void AOnlinePlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	OnlinePlayer = OnlinePlayer == nullptr ? Cast<AMyPlayer>(GetPawn()) : OnlinePlayer;
	if(OnlinePlayer)
	{
		Controller = Controller ==nullptr ? Cast<AOnlinePlayerController>(OnlinePlayer->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
	
	
}

void AOnlinePlayerState::AddToScore(float ScoreAmount)
{
	SetScore(ScoreAmount + GetScore());

	OnlinePlayer = OnlinePlayer == nullptr ? Cast<AMyPlayer>(GetPawn()) : OnlinePlayer;
	if(OnlinePlayer)
	{
		Controller = Controller ==nullptr ? Cast<AOnlinePlayerController>(OnlinePlayer->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AOnlinePlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	OnlinePlayer = OnlinePlayer == nullptr ? Cast<AMyPlayer>(GetPawn()) : OnlinePlayer;
	if(OnlinePlayer)
	{
		Controller = Controller ==nullptr ? Cast<AOnlinePlayerController>(OnlinePlayer->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AOnlinePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOnlinePlayerState,Defeats);
}

void AOnlinePlayerState::OnRep_Defeats()
{
	OnlinePlayer = OnlinePlayer == nullptr ? Cast<AMyPlayer>(GetPawn()) : OnlinePlayer;
	if(OnlinePlayer)
	{
		Controller = Controller ==nullptr ? Cast<AOnlinePlayerController>(OnlinePlayer->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}



