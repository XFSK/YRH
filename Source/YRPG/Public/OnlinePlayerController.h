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

	void SetHUDMatchCountdown(float CountdownTime);

	virtual void Tick(float DeltaSeconds) override;

	virtual float GetServerTime();

	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDAnnouncement(float CountdownTime);
protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime =0.f;

	void CheckTimeSync(float DeltaTime);

	void PollInit();

	void HandleMatchHasStarted();

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client,Reliable)
	void ClientJoinMidgame(FName StateOfMatch,float Warmup,float Match,float StaringTime);
private:
	UPROPERTY()
	class AOnlinePlayerHUD* OnlinePlayerHUD;

	float LevelStartingTime = 0.f;
	
	float MatchTime = 0.f;

	float WarmupTime = 0.f;
	
	uint32 CountdownInt = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

    UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UPlayerOverlap* PlayerOverlap;

	bool bInitializePlayerOverlap = false;

	float HUDHealth;

	float HUDMaxHealth;

	float HUDStamina;

	float HUDMaxStamina;

	float HUDScore;

	int32 HUDDefeats;
};
