// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PoolWorldSubsystem.generated.h"

/**
 * 아이템박스 풀링을 관리하는 월드 서브시스템
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class DUNGEONKITCHEN_API UPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PostInitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// 아이템박스 풀에서 가져오기
	UFUNCTION(BlueprintCallable)
	class ADropItemBox* SpawnDropItemBox(const FVector& SpawnLocation,  int32 ItemID,  int32 Count) ;

protected:
	// 드롭 아이템박스 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Pool Settings")
	TSubclassOf<class ADropItemBox> DropItemClass;

	// 풀 크기
	UPROPERTY(EditDefaultsOnly, Category = "Pool Settings")
	int32 PoolSize = 20;

	// 아이템박스 풀
	UPROPERTY()
	TArray<TObjectPtr<ADropItemBox>> ItemBoxPool;



private:
	// 풀 초기화
	void InitializeItemBoxPool();

	// 풀에서 아이템박스 가져오기
	class ADropItemBox* GetItemBoxFromPool() ;

	// 새 아이템박스 생성
	class ADropItemBox* CreateNewItemBox();
};
