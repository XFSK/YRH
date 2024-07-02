// Fill out your copyright notice in the Description page of Project Settings.


#include "HostGameMode.h"
#include "GameFramework/GameStateBase.h"

void AHostGameMode::PostLogin(APlayerController* NewPlayer)
{
    AGameMode::PostLogin(NewPlayer);
   int32 NumberOfPlayer =  GameState.Get()->PlayerArray.Num();
    if(NumberOfPlayer == 2)
    {
        UWorld* World = GetWorld();
        if(World)
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(FString("/Game/ThirdPerson/Maps/HostMap?listen"));
        }
    }
}

void AHostGameMode::Logout(AController* Exiting)
{
    AGameMode::Logout(Exiting);
}
