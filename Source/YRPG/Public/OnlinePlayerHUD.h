// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OnlinePlayerHUD.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	
	class UTexture2D* CrosshairsLeft;

	class UTexture2D* CrosshairsRight;

	class UTexture2D* CrosshairsTop;

	class UTexture2D* CrosshairsBottom;

	float CrosshairSpread;

	FLinearColor CrosshairsColor;
};


UCLASS()
class YRPG_API AOnlinePlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UUserWidget> PlayerOverlapClass;
	
	class UPlayerOverlap* PlayerOverlap;
	
private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture,FVector2D ViewportCenter,FVector2D Spread,FLinearColor CrosshairsColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;

	void AddPlayerOverlap();
};
