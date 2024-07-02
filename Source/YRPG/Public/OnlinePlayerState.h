// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OnlinePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class YRPG_API AOnlinePlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	
	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);

	void AddToDefeats(int32 DefeatsAmount);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Defeats();
	
private:
	UPROPERTY()
	class AMyPlayer* OnlinePlayer;

	UPROPERTY()
	class AOnlinePlayerController* Controller;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	
};
