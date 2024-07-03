// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOverlap.generated.h"

/**
 * 
 */
UCLASS()
class YRPG_API UPlayerOverlap : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UProgressBar* StaminaBar;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* DefeatsAmount;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* MatchCountdownText;
};
