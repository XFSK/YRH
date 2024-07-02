// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnTakeAnyDamage,UHealthComponent*,HealthComponent,float,Health,float,HealthDelta,const class UDamageType* ,DamageType, class AController*, InstigatedBy, AActor* ,DamageCauser);
UCLASS(BlueprintType,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YRPG_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float CurrentHealth;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float MaxStamina;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float CurrentStamina;
	
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	ACharacter* MyOwner;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	bool isDead;

	UPROPERTY(BlueprintAssignable)
	FOnTakeAnyDamage TakeDamage;
};
