// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"



UCLASS()
class DUNGEONKITCHEN_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle SpawnData;


public:
	UFUNCTION(BlueprintCallable)
	void Spawn();

	void EnemySpawn();
	void CheckNextSpawn();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnCount = 0;

	UPROPERTY(VisibleAnywhere)
	int32 EnemyCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnDelayTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDelaySpawning = false;



	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<class AClearActor>> ClearActor;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//TArray<TObjectPtr<USceneComponent>> SpawnComponents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<AActor>> SpawnActors;

private:
	UFUNCTION()
	void EnemyDie(AActor* Enemy);

	UPROPERTY(VisibleAnywhere)
	bool bIsSpawned = false;


	struct FDKEnemySpawnData* Data;

};
