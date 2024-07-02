// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HostGameMode.generated.h"

/**
 * 
 */
UCLASS()
class YRPG_API AHostGameMode : public AGameMode
{
	GENERATED_BODY()

public:
    virtual void PostLogin(APlayerController* NewPlayer) override;

    virtual void Logout(AController* Exiting) override;
};
