// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/PoolWorldSubsystem.h"
#include "Actor/Other/DropItem/DropItemBox.h"

void UPoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

}

void UPoolWorldSubsystem::PostInitialize()
{
	Super::PostInitialize();


}

void UPoolWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 풀 초기화 (20개의 아이템박스 미리 생성)
	InitializeItemBoxPool();
	UE_LOG(LogTemp, Warning, TEXT("UPoolWorldSubsystem::OnWorldBeginPlay : %s") , *GetName());
}

void UPoolWorldSubsystem::InitializeItemBoxPool()
{
	if (!DropItemClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItemClass is not set!"));
		return;
	}

	// 풀 초기화
	ItemBoxPool.Empty();

	// 20개의 아이템박스 미리 생성
	for (int32 i = 0; i < PoolSize; i++)
	{
		ADropItemBox* NewItemBox = CreateNewItemBox();
		if (NewItemBox)
		{
			// 풀에 추가하고 비활성화
			ItemBoxPool.Add(NewItemBox);
			NewItemBox->SetActorHiddenInGame(true);
			NewItemBox->SetActorEnableCollision(false);

			UE_LOG(LogTemp, Log, TEXT("Created ItemBox %d for pool"), i + 1);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ItemBox Pool initialized with %d items"), ItemBoxPool.Num());
}

ADropItemBox* UPoolWorldSubsystem::CreateNewItemBox()
{
	if (!DropItemClass)
	{
		return nullptr;
	}

	// 월드에 아이템박스 생성 (숨겨진 상태로)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 월드의 중앙에 생성 (나중에 위치 조정)
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	ADropItemBox* NewItemBox = GetWorld()->SpawnActor<ADropItemBox>(DropItemClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (NewItemBox)
	{
		ItemBoxPool.Add(NewItemBox);
		// 풀에 속해있다는 표시
		// (ADropItemBox에 bIsInPool 변수가 있다고 가정)
		// NewItemBox->bIsInPool = true;
	}

	return NewItemBox;
}

ADropItemBox* UPoolWorldSubsystem::GetItemBoxFromPool()
{
	ADropItemBox* FindItem = nullptr;
	// 풀에서 아이템박스 가져오기
	for (ADropItemBox* ItemBox : ItemBoxPool)
	{
		if (ItemBox->IsHidden())
		{
			FindItem = ItemBox;
			break;

		}
	}

	if (FindItem == nullptr)
	{
		// 풀이 비었으면 새로 생성
		UE_LOG(LogTemp, Warning, TEXT("Pool is empty, creating new ItemBox"));
		FindItem = CreateNewItemBox();
	}

	// 활성화
	FindItem->SetActorHiddenInGame(false);
	FindItem->SetActorEnableCollision(true);

	return  FindItem;
}

ADropItemBox* UPoolWorldSubsystem::SpawnDropItemBox(const FVector& SpawnLocation, int32 ItemID, int32 Count)
{
	// 풀에서 아이템박스 가져오기
	ADropItemBox* ItemBox = GetItemBoxFromPool();

	if (ItemBox)
	{
		// 위치 설정

		ItemBox->SetActorLocation(SpawnLocation);
		ItemBox->SetItemBox(ItemID , Count);
		// 아이템 정보 설정 (ADropItemBox에 해당 함수가 있다고 가정)
		// ItemBox->SetItemInfo(ItemIID, Count);


		return ItemBox;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to get ItemBox from pool"));
	return nullptr;
}


