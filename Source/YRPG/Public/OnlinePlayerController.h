// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OnlinePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class YRPG_API AOnlinePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health,float MaxHealth);

	void SetHUDStamina(float Stamina,float MaxStamina);

	void SetHUDScore(float Score);

	void SetHUDDefeats(int32 Defeats);

	void SetHUDWeaponAmmo(int32 Ammo);
	
	virtual void OnPossess(APawn* InPawn) override;

	void SetHUDCarriedAmmo(int32 Ammo);
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class AOnlinePlayerHUD* OnlinePlayerHUD;
};
