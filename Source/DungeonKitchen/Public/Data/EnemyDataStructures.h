// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "EnemyDataStructures.generated.h"




USTRUCT(BlueprintType)
struct FDKEnemyData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer TagContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemyID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> Enemy_BP_Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HP = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> DropItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> DropItemRate;
};

USTRUCT(BlueprintType)
struct FEnemySpawnPos
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemyID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnPointIndex = -1;
};

USTRUCT(BlueprintType)
struct FDKSpawnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnPos> SpawnEnemyArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnTime = 0.0f;


};

USTRUCT(BlueprintType)
struct FDKEnemySpawnData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDKSpawnData> SpawnDataArray;

};


/**
 *
 */

UCLASS()
class DUNGEONKITCHEN_API UEnemyDataStructures : public UObject
{
	GENERATED_BODY()

};
