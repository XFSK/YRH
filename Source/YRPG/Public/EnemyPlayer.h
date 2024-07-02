// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyPlayer.generated.h"

UCLASS()
class YRPG_API AEnemyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bCombat;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class AWeapon* CurrentWeapon;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	class UAIPerceptionComponent* AiComponent;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UHealthComponent* HealthComponent;

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* CameraComponent;
	
	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable)
	void EnemyFire();

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageType;
};
