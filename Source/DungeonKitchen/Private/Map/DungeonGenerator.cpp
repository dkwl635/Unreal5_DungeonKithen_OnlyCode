// Copyright © 2025 Tartare Studio

#include "Map/DungeonGenerator.h"
#include "Map/RoomBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

// 생성자
ADungeonGenerator::ADungeonGenerator()
{
	// 틱 비활성화 (성능 최적화)
	PrimaryActorTick.bCanEverTick = false;
}

// 게임 시작 시 호출
void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	// 던전 생성 시작
	StartDungeonGeneration();
}

// 던전 생성을 시작하는 함수
void ADungeonGenerator::StartDungeonGeneration()
{
	// 랜덤 시드 설정
	FMath::RandInit(RandomSeed);

	// 고급 생성 모드에 따라 분기
	if (bUseAdvancedGeneration)
	{
		StartAdvancedDungeonGeneration();
	}
	else
	{
		StartLegacyDungeonGeneration();
	}
}

// 고급 던전 생성 함수
void ADungeonGenerator::StartAdvancedDungeonGeneration()
{
	UE_LOG(LogTemp, Log, TEXT("=== 고급 던전 생성 시작 ==="));
	UE_LOG(LogTemp, Log, TEXT("제너레이터 위치: %s"), *GetActorTransform().GetLocation().ToString());
	UE_LOG(LogTemp, Log, TEXT("중간 방 개수: %d"), MiddleRoomCount);
	UE_LOG(LogTemp, Log, TEXT("중간 방 풀 크기: %d"), MiddleRoomPool.Num());

	// 유효성 검사
	if (!StartRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartRoom이 설정되지 않았습니다"));
		return;
	}
	if (!EndRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("EndRoom이 설정되지 않았습니다"));
		return;
	}
	if (MiddleRoomPool.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MiddleRoomPool이 비어있습니다"));
		return;
	}

	// Actor 스폰 파라미터 설정 (전체 함수에서 재사용)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 1. 첫 번째 방 생성 (고정) - Generator 위치에 직접 스폰
	UE_LOG(LogTemp, Log, TEXT("=== 첫 번째 방 생성 ==="));
	
	ARoomBase* FirstRoom = GetWorld()->SpawnActor<ARoomBase>(StartRoom, GetActorTransform(), SpawnParams);
	if (!FirstRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("첫 번째 방 생성에 실패했습니다"));
		return;
	}

	// 첫 번째 방의 출구 위치를 다음 방 연결의 시작점으로 설정
	FTransform FirstEntranceLocal, FirstExitWorld;
	if (GetArrowTransforms(FirstRoom, FirstEntranceLocal, FirstExitWorld))
	{
		CurrentExitWorld = FirstExitWorld;
		SpawnedRooms.Add(FirstRoom);

		// 첫 번째 방 위치 확정 후 Level Streaming 시작
		FirstRoom->LoadRoomLevelAsync();
		UE_LOG(LogTemp, Log, TEXT("첫 번째 방 위치 확정 후 Level Streaming 시작"));

		UE_LOG(LogTemp, Log, TEXT("첫 번째 방 Exit 위치: %s"), *CurrentExitWorld.GetLocation().ToString());

		if (bDrawDebug)
		{
			DrawLink(FirstRoom->GetActorTransform(), CurrentExitWorld, FColor::Green);
		}
	}

	// 2. 중간 방들 생성 (새로운 Level Streaming 방식)
	UE_LOG(LogTemp, Log, TEXT("=== 중간 방들 생성 ==="));
	for (int32 i = 0; i < MiddleRoomCount; ++i)
	{
		// 중간 방 풀에서 랜덤 선택
		int32 RandomIndex = FMath::RandRange(0, MiddleRoomPool.Num() - 1);
		TSubclassOf<ARoomBase> SelectedRoomClass = MiddleRoomPool[RandomIndex];

		UE_LOG(LogTemp, Log, TEXT("=== %d번째 중간 방 생성 (풀에서 %d번째 방 선택) ==="), i + 1, RandomIndex + 1);

		// 하이브리드 방식: SpawnAlignedRoom으로 위치 연결 + Level Streaming
		ARoomBase* MiddleRoom = SpawnAlignedRoom(SelectedRoomClass, CurrentExitWorld);
		if (MiddleRoom)
		{
			FTransform MiddleEntranceLocal, MiddleExitWorld;
			if (GetArrowTransforms(MiddleRoom, MiddleEntranceLocal, MiddleExitWorld))
			{
				CurrentExitWorld = MiddleExitWorld;
				SpawnedRooms.Add(MiddleRoom);

				UE_LOG(LogTemp, Log, TEXT("중간 방 %d 생성 완료, Exit 위치: %s"), i + 1, *CurrentExitWorld.ToString());

				if (bDrawDebug)
				{
					DrawLink(MiddleRoom->GetActorTransform(), CurrentExitWorld, FColor::Blue);
				}
			}
		}
	}

	// 3. 마지막 방 생성 (기존 방식)
	UE_LOG(LogTemp, Log, TEXT("=== 마지막 방 생성 ==="));
	ARoomBase* LastRoom = SpawnAlignedRoom(EndRoom, CurrentExitWorld);
	if (LastRoom)
	{
		FTransform LastEntranceLocal, LastExitWorld;
		if (GetArrowTransforms(LastRoom, LastEntranceLocal, LastExitWorld))
		{
			SpawnedRooms.Add(LastRoom);

			UE_LOG(LogTemp, Log, TEXT("마지막 방 생성 완료, Exit 위치: %s"), *LastExitWorld.ToString());

			if (bDrawDebug)
			{
				DrawLink(LastRoom->GetActorTransform(), LastExitWorld, FColor::Red);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=== 고급 던전 생성 완료 ==="));
	UE_LOG(LogTemp, Log, TEXT("총 생성된 방 개수: %d"), SpawnedRooms.Num());
}

// 레거시 던전 생성 함수
void ADungeonGenerator::StartLegacyDungeonGeneration()
{
	// 방 시퀀스가 비어있으면 경고 후 리턴
	if (RoomSequence.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoomSequence가 비어있습니다"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== 레거시 던전 생성 시작 ==="));
	UE_LOG(LogTemp, Log, TEXT("제너레이터 위치: %s"), *GetActorTransform().GetLocation().ToString());
	UE_LOG(LogTemp, Log, TEXT("총 방 개수: %d"), RoomSequence.Num());

	// 첫 번째 방을 제너레이터 위치에 스폰
	ARoomBase* FirstRoom = SpawnAlignedRoom(RoomSequence[0], GetActorTransform());
	if (!FirstRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("첫 번째 방 생성에 실패했습니다"));
		return;
	}

	// 첫 번째 방의 출구 월드 변환을 현재 출구로 설정
	FTransform FirstExitLocal, FirstExitWorld;
	if (GetArrowTransforms(FirstRoom, FirstExitLocal, FirstExitWorld))
	{
		CurrentExitWorld = FirstExitWorld;
		SpawnedRooms.Add(FirstRoom);

		if (bDrawDebug)
		{
			DrawLink(FirstRoom->GetActorTransform(), CurrentExitWorld, FColor::Blue);
		}
	}

	// 나머지 방들을 순서대로 생성
	for (int32 i = 1; i < RoomSequence.Num(); ++i)
	{
		ARoomBase* NextRoom = SpawnAlignedRoom(RoomSequence[i], CurrentExitWorld);
		if (NextRoom)
		{
			FTransform NextEntranceLocal, NextExitWorld;
			if (GetArrowTransforms(NextRoom, NextEntranceLocal, NextExitWorld))
			{
				CurrentExitWorld = NextExitWorld;
				SpawnedRooms.Add(NextRoom);

				if (bDrawDebug)
				{
					DrawLink(NextRoom->GetActorTransform(), CurrentExitWorld, FColor::Blue);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=== 레거시 던전 생성 완료 ==="));
	UE_LOG(LogTemp, Log, TEXT("총 생성된 방 개수: %d"), SpawnedRooms.Num());
}



// 방을 정렬하여 스폰하는 함수
ARoomBase* ADungeonGenerator::SpawnAlignedRoom(TSubclassOf<ARoomBase> RoomClass, const FTransform& TargetExitWorld, bool bIsEndRoom)
{
	// 클래스가 유효하지 않으면 리턴
	if (!RoomClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoomClass가 유효하지 않습니다"));
		return nullptr;
	}

	// 월드가 유효하지 않으면 리턴
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("월드가 유효하지 않습니다"));
		return nullptr;
	}

	// 임시 위치에서 스폰하여 EntranceLocal을 가져온 후 계산
	FTransform TempTransform = FTransform::Identity;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARoomBase* SpawnedRoom = World->SpawnActor<ARoomBase>(RoomClass, TempTransform, SpawnParams);
	if (!SpawnedRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("방 스폰에 실패했습니다"));
		return nullptr;
	}

	// 방의 입구 로컬 변환 가져오기
	FTransform EntranceLocal, ExitWorld;
	if (GetArrowTransforms(SpawnedRoom, EntranceLocal, ExitWorld))
	{
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: EntranceLocal = %s"), *EntranceLocal.GetLocation().ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: TargetExitWorld = %s"), *TargetExitWorld.GetLocation().ToString());

		// 방법 1: 생성 후 위치 조정 (가장 확실한 방법)
		// 1. 방을 임시 위치에 생성 (이미 완료됨)

		// 2. Exit의 회전을 기준으로 새 방의 회전 계산
		// 새 방의 Entrance가 이전 방의 Exit와 같은 방향을 바라보도록 설정
		FRotator ExitRotation = TargetExitWorld.GetRotation().Rotator();
		ExitRotation.Pitch = 0.0f;  // 평면 연결
		ExitRotation.Roll = 0.0f;   // 평면 연결
		
		// Entrance가 Exit과 같은 방향을 바라보도록
		// 모든 방이 같은 방향으로 연결됨
		
		UE_LOG(LogTemp, Log, TEXT("새 방 회전 설정: %s"), *ExitRotation.ToString());
		
		SpawnedRoom->SetActorRotation(ExitRotation);

		// 3. 회전 적용 후 Entrance의 월드 위치를 가져옴
		FVector CurrentEntranceWorld = SpawnedRoom->Entrance->GetComponentTransform().GetLocation();
		FVector TargetExitLocation = TargetExitWorld.GetLocation();

		// 4. Entrance를 Exit 위치에 정확히 맞추기 위한 오프셋 계산
		FVector Offset = TargetExitLocation - CurrentEntranceWorld;

		// 5. 방을 오프셋만큼 이동
		FVector CurrentRoomLocation = SpawnedRoom->GetActorLocation();
		FVector NewLocation = CurrentRoomLocation + Offset;
		SpawnedRoom->SetActorLocation(NewLocation);

		// 6. 이동 후 실제 Entrance 위치 확인
		FVector FinalEntranceWorld = SpawnedRoom->Entrance->GetComponentTransform().GetLocation();

		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 현재 Entrance 위치 = %s"), *CurrentEntranceWorld.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 목표 Exit 위치 = %s"), *TargetExitLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: Exit 회전 = %s"), *ExitRotation.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 계산된 오프셋 = %s"), *Offset.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 현재 방 위치 = %s"), *CurrentRoomLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 새로운 방 위치 = %s"), *NewLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 이동 후 Entrance 위치 = %s"), *FinalEntranceWorld.ToString());

		// 연결 검증을 위한 추가 로그
		FVector ConnectionError = FinalEntranceWorld - TargetExitLocation;
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: 연결 오차 = %.3f"), ConnectionError.Size());

		// Room Actor 위치 확정 후 Level Streaming 시작
		SpawnedRoom->LoadRoomLevelAsync();
		UE_LOG(LogTemp, Log, TEXT("SpawnAlignedRoom: Room Actor 위치 확정 후 Level Streaming 시작"));

		return SpawnedRoom;
	}

	// 실패 시 스폰된 액터 제거
	SpawnedRoom->Destroy();
	return nullptr;
}


// 화살표 변환 정보를 가져오는 정적 함수
bool ADungeonGenerator::GetArrowTransforms(ARoomBase* Room, FTransform& OutEntranceLocal, FTransform& OutExitWorld)
{
	// ARoomBase의 정적 함수 호출
	return ARoomBase::GetArrowTransforms(Room, OutEntranceLocal, OutExitWorld);
}

// 연결선을 그리는 함수
void ADungeonGenerator::DrawLink(const FTransform& A, const FTransform& B, const FColor& Color)
{
	// 월드가 유효하지 않으면 리턴
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 연결선 그리기 (10초간 표시)
	DrawDebugLine(World, A.GetLocation(), B.GetLocation(), Color, false, 10.0f, 0, 3.0f);

	// 시작점과 끝점 그리기 (더 큰 점으로)
	DrawDebugPoint(World, A.GetLocation(), 15.0f, Color, false, 10.0f, 0);
	DrawDebugPoint(World, B.GetLocation(), 15.0f, Color, false, 10.0f, 0);

	// 연결 거리 표시
	FVector ConnectionVector = B.GetLocation() - A.GetLocation();
	float Distance = ConnectionVector.Size();

	// 중간 지점에 거리 텍스트 표시
	FVector MidPoint = A.GetLocation() + (ConnectionVector * 0.5f);
	DrawDebugString(World, MidPoint, FString::Printf(TEXT("%.2f"), Distance), nullptr, FColor::White, 10.0f);
}

// Room의 Exit 위치가 업데이트될 때 호출되는 함수
void ADungeonGenerator::OnRoomExitUpdated(ARoomBase* Room, FTransform ExitTransform)
{
	if (!Room)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ADungeonGenerator::OnRoomExitUpdated - Room: %s, Exit 위치: %s"), 
		*Room->GetName(), *ExitTransform.ToString());

	// 다음 방이 있다면 연결 정보 업데이트
	int32 RoomIndex = SpawnedRooms.Find(Room);
	if (RoomIndex != INDEX_NONE && RoomIndex + 1 < SpawnedRooms.Num())
	{
		ARoomBase* NextRoom = SpawnedRooms[RoomIndex + 1];
		if (NextRoom)
		{
			// 다음 방에 정확한 연결 정보 전달
			NextRoom->SetConnectionTarget(ExitTransform);
			UE_LOG(LogTemp, Log, TEXT("ADungeonGenerator::OnRoomExitUpdated - 다음 방에 연결 정보 업데이트"));
		}
	}
}


