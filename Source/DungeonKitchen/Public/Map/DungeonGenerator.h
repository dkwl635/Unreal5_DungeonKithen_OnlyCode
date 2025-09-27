// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/RoomBase.h"
#include "DungeonGenerator.generated.h"

// 모듈 플로우 열거형
UENUM(BlueprintType)
enum class EModuleFlow : uint8
{
	FixedSequence,  // 고정 시퀀스 모드
	RandomSequence  // 랜덤 시퀀스 모드
};

/**
 * 방들을 Exit(부모) = Entrance(자식) 정렬로 순차 배치하는 던전 제너레이터
 * Start → Combat×N → (옵션 Reward) → Boss 순서로 방을 생성
 */
UCLASS(BlueprintType, Blueprintable)
class DUNGEONKITCHEN_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// 생성자
	ADungeonGenerator();

protected:
	// 게임 시작 시 호출
	virtual void BeginPlay() override;

public:	
	// === 고급 던전 생성 설정 ===
	// 첫 번째 방 (고정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Rooms")
	TSubclassOf<ARoomBase> StartRoom;

	// 마지막 방 (고정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Rooms")
	TSubclassOf<ARoomBase> EndRoom;

	// 중간 방 풀 (랜덤 선택용) - 원하는 만큼 방 종류 추가 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Rooms", meta = (TitleProperty = "Room Class"))
	TArray<TSubclassOf<ARoomBase>> MiddleRoomPool;

	// 중간 방 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Flow", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MiddleRoomCount = 5;

	// === 레거시 설정 (호환성 유지) ===
	// 방 시퀀스 배열 (순서대로 생성: 전투-리워드-보스 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy", meta = (EditCondition = "!bUseAdvancedGeneration"))
	TArray<TSubclassOf<ARoomBase>> RoomSequence;

	// 시퀀스 모드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy", meta = (EditCondition = "!bUseAdvancedGeneration"))
	EModuleFlow SequenceMode = EModuleFlow::FixedSequence;

	// 고급 생성 모드 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Flow")
	bool bUseAdvancedGeneration = true;

	// 랜덤 시드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int32 RandomSeed = 12345;

	// 디버그 그리기 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = true;



protected:
	// 방을 정렬하여 스폰하는 함수 (단일 출구용)
	UFUNCTION(BlueprintCallable, Category = "Dungeon Generation")
	ARoomBase* SpawnAlignedRoom(TSubclassOf<ARoomBase> RoomClass, const FTransform& TargetExitWorld, bool bIsEndRoom = false);


	// 화살표 변환 정보를 가져오는 정적 함수
	UFUNCTION(BlueprintCallable, Category = "Dungeon Utils")
	static bool GetArrowTransforms(ARoomBase* Room, FTransform& OutEntranceLocal, FTransform& OutExitWorld);

	// 연결선을 그리는 함수
	UFUNCTION(BlueprintCallable, Category = "Dungeon Debug")
	void DrawLink(const FTransform& A, const FTransform& B, const FColor& Color);

	// 던전 생성을 시작하는 함수
	void StartDungeonGeneration();

	// 고급 던전 생성 함수
	void StartAdvancedDungeonGeneration();

	// 레거시 던전 생성 함수
	void StartLegacyDungeonGeneration();
	
	// Room의 Exit 위치가 업데이트될 때 호출되는 함수
	UFUNCTION()
	void OnRoomExitUpdated(ARoomBase* Room, FTransform ExitTransform);

private:
	// 현재 출구 월드 변환 (다음 방의 입구 위치)
	FTransform CurrentExitWorld;
	
	// 생성된 방들의 배열
	UPROPERTY()
	TArray<ARoomBase*> SpawnedRooms;
};
