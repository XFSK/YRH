// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "PackageObject.generated.h"

UENUM(BlueprintType,Blueprintable)
enum class EPackageEnum : uint8
{
	Magic,
	Life,
	Rifle,
	HandGun,
};


USTRUCT(BlueprintType,Blueprintable)
struct FPackageData :public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FName ObjectName;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
    int32 Index;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UTexture2D* ObjectThumbnail;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USkeletalMesh* ObjectMesh;

    UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UStaticMesh* ObjectStaticMesh;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsStacking;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 Count;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 MaxCount;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EPackageEnum Class;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bIsEmpty;
};





UCLASS()
class YRPG_API APackageObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APackageObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
