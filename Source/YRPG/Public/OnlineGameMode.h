// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OnlineGameMode.generated.h"

/**
 * 
 */
UCLASS()
class YRPG_API AOnlineGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AMyPlayer* ElimmedPlayer,class AOnlinePlayerController* VictimController,AOnlinePlayerController* AttackerController);

	virtual void RequestRespawn(class AMyPlayer* ElimmedPlayer,AController* ElimmedController);
};
