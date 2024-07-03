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
    AOnlineGameMode();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PlayerEliminated(class AMyPlayer* ElimmedPlayer,class AOnlinePlayerController* VictimController,AOnlinePlayerController* AttackerController);

	virtual void RequestRespawn(class AMyPlayer* ElimmedPlayer,AController* ElimmedController);

	UPROPERTY(EditAnywhere)
	float WarmupTime = 10.f;

	UPROPERTY(EditAnywhere)
	float MatchTime = 120.f;
	
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;
private:
	float CountdownTime =0.f;
};
