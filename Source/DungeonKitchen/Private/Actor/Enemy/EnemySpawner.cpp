// Copyright © 2025 Tartare Studio


#include "Actor/Enemy/EnemySpawner.h"

#include "Actor/Other/ClearActor.h"
#include "Character/Enemy/BossBase.h"
#include "Character/Enemy/DKEnemy.h"
#include "Components/BoxComponent.h"
#include "Data/EnemyDataStructures.h"
#include "Game/Subsystem/DataManager.h"
#include "Interface/RoomInterface.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	Data = SpawnData.GetRow<FDKEnemySpawnData>(TEXT(""));
	if (Data == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FDKEnemySpawnData GetRow is NULL"));
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDelaySpawning)
	{

		SpawnDelayTimer -= DeltaTime;
		if (SpawnDelayTimer < 0)
		{
			bIsDelaySpawning = false;
			Spawn();
		}
	}
}





void AEnemySpawner::Spawn()
{
	if (Data == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("FDKEnemySpawnData GetRow is NULL"));
		return;
	}

	EnemySpawn();
	CheckNextSpawn();
}

void AEnemySpawner::EnemySpawn()
{
	Data = SpawnData.GetRow<FDKEnemySpawnData>(TEXT(""));
	if (Data->SpawnDataArray.Num() <= SpawnCount)
	{
		//모든 정보 스폰 완료
 		UE_LOG(LogTemp, Warning, TEXT("All Clear"));
		return;
	}

	FDKSpawnData& CurrentData = Data->SpawnDataArray[SpawnCount];
	for (FEnemySpawnPos& EnemySpawnData :  CurrentData.SpawnEnemyArray)
	{
		int32 EnemyID = EnemySpawnData.EnemyID;
		//FVector SpawnLocation = EnemySpawnData.SpawnLocation;

		// SpawnActors 배열 범위 체크
		if (SpawnActors.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnActors array is empty!"));
			continue;
		}

		if (EnemySpawnData.SpawnPointIndex >= SpawnActors.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnPointIndex %d is out of range! SpawnActorComponents.Num() = %d"),
				EnemySpawnData.SpawnPointIndex, SpawnActors.Num());
			continue;
		}

		if (!SpawnActors[EnemySpawnData.SpawnPointIndex])
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnActors[%d] is null!"), EnemySpawnData.SpawnPointIndex);
			continue;
		}

		FVector SpawnLocation = SpawnActors[EnemySpawnData.SpawnPointIndex]->GetActorLocation();

		UDataManager* DataManager=	GetGameInstance()->GetSubsystem<UDataManager>();
		if (!DataManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("DataManager is NULL"));
			return;
		}

		FDKEnemyData EnemyData;
		if (!DataManager->GetEnemyData(EnemyID,EnemyData))
		{
			UE_LOG(LogTemp, Warning , TEXT("EnemyData Null"));
			return;
		}
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		SpawnTransform.SetRotation(FQuat(FRotator(0,0,0)));

		if (EnemyData.Enemy_BP_Class->IsChildOf(ADKEnemy::StaticClass()))
		{
			ADKEnemy* Enemy = GetWorld()->SpawnActorDeferred<ADKEnemy>(EnemyData.Enemy_BP_Class, SpawnTransform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (!Enemy)
			{
				UE_LOG(LogTemp , Warning, TEXT("Enemy Spawn Failed"));
				return;
			}
			Enemy->EnemyID = EnemyID;
			Enemy->OnEnemyDieEvent.AddDynamic(this, &AEnemySpawner::EnemyDie);
			// 초기화 다 끝난 후에 실제로 월드에 스폰 완료
			UGameplayStatics::FinishSpawningActor(Enemy, SpawnTransform);

			EnemyCount++;
		}
		else
		{
			ABossBase* Boss = GetWorld()->SpawnActorDeferred<ABossBase>(EnemyData.Enemy_BP_Class, SpawnTransform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (!Boss)
			{
				UE_LOG(LogTemp , Warning, TEXT("Enemy Spawn Failed"));
				return;
			}

			Boss->BossID = EnemyID;
			Boss->OnBossDieEvent.AddDynamic(this, &AEnemySpawner::EnemyDie);
			// 초기화 다 끝난 후에 실제로 월드에 스폰 완료
			UGameplayStatics::FinishSpawningActor(Boss, SpawnTransform);

			EnemyCount++;
		}

	}

	SpawnCount++;
}

void AEnemySpawner::CheckNextSpawn()
{
	if (!Data) return;

	if (Data->SpawnDataArray.Num() <= SpawnCount)
	{
		//모든 정보 스폰 완료
		UE_LOG(LogTemp, Warning, TEXT("No SpawnDataArray"));
		return;
	}

	FDKSpawnData& CurrentData = Data->SpawnDataArray[SpawnCount];
	SpawnDelayTimer = CurrentData.SpawnTime;
	bIsDelaySpawning = true;
}

void AEnemySpawner::EnemyDie(AActor* Enemy)
{
	EnemyCount--;

	if (EnemyCount == 0)
	{
		//현제 예약중인게 있으면 취소
		bIsDelaySpawning = false;

		if (Data->SpawnDataArray.Num() > SpawnCount)
		{
			Spawn();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Clear"));
			for (AClearActor* Actor : ClearActor)
			{
				if (Actor)
				{
					Actor->RoomClear();
				}
			}

		}
	}


}
