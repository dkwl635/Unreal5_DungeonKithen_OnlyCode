// Copyright © 2025 Tartare Studio

#include "Map/RoomBase.h"
#include "Components/ArrowComponent.h"
#include "Engine/Engine.h"

// 정적 상수 정의 (성능 최적화)
const FTransform ARoomBase::R180Transform = FTransform(FRotator(0.0f, 180.0f, 0.0f));

// 생성자
ARoomBase::ARoomBase()
{
	// 틱 비활성화 (성능 최적화)
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 설정
	SetupComponents();

	// 로드된 레벨 초기화
	LoadedStreamingLevel = nullptr;
	bWaitingForLevelLoad = false;
	PendingTargetExitWorld = FTransform::Identity;

	// 캐시된 앵커 초기화
	CachedEntranceActor = nullptr;
	CachedExitActor = nullptr;

	// Cleanup 재시도 횟수 초기화
	CleanupRetryCount = 0;
}

// 게임 시작 시 호출
void ARoomBase::BeginPlay()
{
	Super::BeginPlay();

	// 메인 레벨 보호: 게임 시작 시 Temp 폴더 액터 생성 방지
	if (IsMainLevel())
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::BeginPlay - 메인 레벨에서 시작합니다. Temp 폴더 액터 생성 방지를 적용합니다."));
		PreventTempActorCreation();
	}

	// Room Level 로드는 위치가 확정된 후에 수동으로 호출
	// LoadRoomLevelAsync(); // BeginPlay에서는 호출하지 않음
}

// 게임 종료 시 호출
void ARoomBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Room Level 언로드
	UnloadRoomLevel();

	Super::EndPlay(EndPlayReason);
}


// 컴포넌트들을 설정하는 함수
void ARoomBase::SetupComponents()
{
	// 기본 루트 컴포넌트 생성 (화살표들이 여기에 직접 붙음)
	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	RootComponent = DefaultRoot;

	// 입구 화살표 컴포넌트 생성 및 설정 (Actor 바로 아래)
	Entrance = CreateDefaultSubobject<UArrowComponent>(TEXT("Entrance"));
	Entrance->SetupAttachment(DefaultRoot); // Actor 바로 아래
	Entrance->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // 기본 위치 (에디터에서 자유 조정)
	Entrance->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // +X 방향
	Entrance->SetArrowColor(FColor::Green); // 초록색
	Entrance->SetHiddenInGame(true); // 게임에서 숨김
	Entrance->SetVisibility(true); // 에디터에서는 보임

	// 출구 화살표 컴포넌트 생성 및 설정
	Exit = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit"));
	Exit->SetupAttachment(DefaultRoot);
	Exit->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Exit->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Exit->SetArrowColor(FColor::Red);
	Exit->SetHiddenInGame(true);
	Exit->SetVisibility(true);
}

// 화살표 변환 정보를 가져오는 정적 함수
bool ARoomBase::GetArrowTransforms(ARoomBase* Room, FTransform& OutEntranceLocal, FTransform& OutExitWorld)
{
	// 널 체크
	if (!Room || !Room->Entrance || !Room->Exit)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetArrowTransforms: Room 또는 화살표 컴포넌트가 유효하지 않습니다"));
		return false;
	}

	// Entrance는 로컬 변환 (Relative)
	OutEntranceLocal = Room->Entrance->GetRelativeTransform();

	// Exit의 월드 변환 (World)
	OutExitWorld = Room->Exit->GetComponentTransform();

	return true;
}

// Room Level을 비동기로 로드하는 함수 (새로운 방식)
void ARoomBase::LoadRoomLevelAsync()
{
	// Room Level이 설정되지 않았으면 리턴
	if (RoomLevel.IsNull())
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::LoadRoomLevelAsync - RoomLevel이 설정되지 않았습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::LoadRoomLevelAsync - World가 nullptr입니다."));
		return;
	}

	// 레벨 경로 가져오기
	FString LevelPath = RoomLevel.GetLongPackageName();
	if (LevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::LoadRoomLevelAsync - RoomLevel 경로가 비어있습니다."));
		return;
	}

	// 메인 레벨 보호: 메인 레벨에서는 Temp 폴더 액터 생성 방지
	if (IsMainLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::LoadRoomLevelAsync - 메인 레벨에서 레벨 로드를 시도합니다. Temp 폴더 액터 생성 방지를 적용합니다."));
		PreventTempActorCreation();
	}

	// 매우 간단한 방식: Room Actor 위치에 바로 로드하고 즉시 가시화
	FTransform WorldTransform = LevelTransform * GetActorTransform();

	bool bSuccess = false;
	LoadedStreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
		this,
		LevelPath,
		WorldTransform.GetLocation(),
		WorldTransform.GetRotation().Rotator(),
		bSuccess
	);

	if (LoadedStreamingLevel && bSuccess)
	{
		// 즉시 로드하고 가시화 (연결 로직 제거)
		LoadedStreamingLevel->SetShouldBeLoaded(true);
		LoadedStreamingLevel->SetShouldBeVisible(true); // 즉시 보이게
		LoadedStreamingLevel->bShouldBlockOnLoad = false; // 비동기 로드

		UE_LOG(LogTemp, Log, TEXT("ARoomBase::LoadRoomLevelAsync - '%s' 로드 및 즉시 가시화"), *LevelPath);

		// Temp 폴더 액터 자동 삭제
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::LoadRoomLevelAsync - CleanupTempActors 호출 전"));
		CleanupTempActors();

		// 델리게이트 즉시 호출
		OnLevelStreamingComplete.Broadcast(this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::LoadRoomLevelAsync - '%s' 스트리밍 로드 실패"), *LevelPath);
	}
}

// Room Level을 언로드하는 함수
void ARoomBase::UnloadRoomLevel()
{
	if (!LoadedStreamingLevel)
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::UnloadRoomLevel - 언로드할 Level이 없습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::UnloadRoomLevel - Room Level 언로드 시작"));

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::UnloadRoomLevel - World가 nullptr입니다."));
		return;
	}

	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(LoadStatusTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CleanupTimerHandle);

	// 레벨 언로드
	LoadedStreamingLevel->SetShouldBeLoaded(false);
	LoadedStreamingLevel->SetShouldBeVisible(false);

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::UnloadRoomLevel - Room Level 언로드 완료"));

	// 레퍼런스 클리어
	LoadedStreamingLevel = nullptr;

	// 캐시된 앵커 정리
	CachedEntranceActor = nullptr;
	CachedExitActor = nullptr;

	// Cleanup 재시도 횟수 리셋
	CleanupRetryCount = 0;
}

// 레벨 로드 완료 시 호출되는 함수 (최적화된 통합 버전)
void ARoomBase::OnRoomLevelLoaded()
{
	if (!LoadedStreamingLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::OnRoomLevelLoaded - LoadedStreamingLevel이 nullptr입니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::OnRoomLevelLoaded - 레벨 로드 완료, 앵커 탐색 시작"));

	// 로드된 레벨에서 앵커 찾기 및 캐싱
	if (!CacheAnchorsFromLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::OnRoomLevelLoaded - 앵커 캐싱 실패"));
		return;
	}

	// 연결 대기 중이면 최종 Transform 적용
	if (bWaitingForLevelLoad)
	{
		ApplyFinalLevelTransform(PendingTargetExitWorld);
		bWaitingForLevelLoad = false;
	}
	else
	{
		// 첫 번째 방인 경우 그냥 가시화
		LoadedStreamingLevel->SetShouldBeVisible(true);
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::OnRoomLevelLoaded - 첫 번째 방 가시화"));
	}

	// 임시: 모든 방을 가시화 (테스트용)
	if (LoadedStreamingLevel)
	{
		LoadedStreamingLevel->SetShouldBeVisible(true);
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::OnRoomLevelLoaded - 임시 가시화 적용"));
	}

	// Exit 위치를 DungeonGenerator에게 알려주기 (다음 방 연결을 위해)
	if (CachedExitActor)
	{
		FTransform ExitAuthoredTransform = CachedExitActor->GetActorTransform();

		// 첫 번째 방이거나 연결 전이면 저작 공간 기준으로 전달
		// 연결된 방들의 정확한 Exit 위치는 ApplyFinalLevelTransform에서 처리
		OnExitLocationUpdated.Broadcast(this, ExitAuthoredTransform);
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::OnRoomLevelLoaded - Exit 위치 업데이트: %s"), *ExitAuthoredTransform.ToString());
	}

	// Temp 폴더 액터 자동 삭제
	UE_LOG(LogTemp, Log, TEXT("ARoomBase::OnRoomLevelLoaded - CleanupTempActors 호출 전"));
	CleanupTempActors();

	// 델리게이트 호출
	OnLevelStreamingComplete.Broadcast(this);
}

// 앵커를 캐시하는 최적화된 함수
bool ARoomBase::CacheAnchorsFromLevel()
{
	if (!LoadedStreamingLevel)
	{
		return false;
	}

	ULevel* LoadedLevel = LoadedStreamingLevel->GetLoadedLevel();
	if (!LoadedLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::CacheAnchorsFromLevel - 로드된 레벨을 가져올 수 없습니다."));
		return false;
	}

	// 캐시 초기화
	CachedEntranceActor = nullptr;
	CachedExitActor = nullptr;

	// 한 번의 순회로 모든 앵커 찾기 (성능 최적화)
	for (AActor* Actor : LoadedLevel->Actors)
	{
		if (!Actor) continue;

		if (Actor->Tags.Contains(FName(TEXT("Entrance"))))
		{
			CachedEntranceActor = Actor;
			UE_LOG(LogTemp, Log, TEXT("ARoomBase::CacheAnchorsFromLevel - Entrance 앵커 발견: %s"), *Actor->GetName());
		}
		else if (Actor->Tags.Contains(FName(TEXT("Exit"))))
		{
			CachedExitActor = Actor;
			UE_LOG(LogTemp, Log, TEXT("ARoomBase::CacheAnchorsFromLevel - Exit 앵커 발견: %s"), *Actor->GetName());
		}
	}

	if (!CachedEntranceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::CacheAnchorsFromLevel - Entrance 앵커를 찾을 수 없습니다."));
		return false;
	}

	if (!CachedExitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::CacheAnchorsFromLevel - Exit 앵커를 찾을 수 없습니다."));
	}

	return true;
}

// 레벨에서 앵커를 찾는 함수
AActor* ARoomBase::FindAnchorInLevel(ULevel* Level, const FString& Tag)
{
	if (!Level)
	{
		return nullptr;
	}

	// 레벨의 모든 액터를 순회하면서 태그로 찾기
	for (AActor* Actor : Level->Actors)
	{
		if (Actor && Actor->Tags.Contains(FName(*Tag)))
		{
			UE_LOG(LogTemp, Log, TEXT("ARoomBase::FindAnchorInLevel - %s 앵커 발견: %s"), *Tag, *Actor->GetName());
			return Actor;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ARoomBase::FindAnchorInLevel - %s 태그를 가진 앵커를 찾을 수 없습니다."), *Tag);
	return nullptr;
}

// 최종 레벨 Transform을 적용하는 함수 (최적화된 버전)
void ARoomBase::ApplyFinalLevelTransform(const FTransform& TargetExitWorld)
{
	if (!LoadedStreamingLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::ApplyFinalLevelTransform - LoadedStreamingLevel이 nullptr입니다."));
		return;
	}

	// 캐시된 앵커 사용 (성능 최적화)
	if (!CachedEntranceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::ApplyFinalLevelTransform - 캐시된 Entrance 앵커가 없습니다."));
		return;
	}

	// 저작 공간에서의 Entrance Transform
	FTransform T_entr_authored = CachedEntranceActor->GetActorTransform();

	// 정렬 공식: T_level = T_exit_prev_world * R(0,180,0) * Inverse(T_entr_authored)
	// 정적 상수 사용 (성능 최적화)
	FTransform T_level = TargetExitWorld * R180Transform * T_entr_authored.Inverse();

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - 계산된 레벨 Transform: %s"), *T_level.ToString());
	UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - Target Exit World: %s"), *TargetExitWorld.ToString());
	UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - Entrance Authored: %s"), *T_entr_authored.ToString());

	// World 한 번만 가져오기 (성능 최적화)
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::ApplyFinalLevelTransform - World가 nullptr입니다."));
		return;
	}

	// 기존 레벨 언로드
	LoadedStreamingLevel->SetShouldBeLoaded(false);
	LoadedStreamingLevel->SetShouldBeVisible(false);

	// 올바른 Transform으로 다시 로드
	FString LevelPath = RoomLevel.GetLongPackageName();

	bool bSuccess = false;
	LoadedStreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
		this,
		LevelPath,
		T_level.GetLocation(),
		T_level.GetRotation().Rotator(),
		bSuccess
	);

	if (LoadedStreamingLevel && bSuccess)
	{
		LoadedStreamingLevel->SetShouldBeLoaded(true);
		LoadedStreamingLevel->SetShouldBeVisible(true);
		LoadedStreamingLevel->bShouldBlockOnLoad = false;

		UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - 레벨 재로드 완료"));

		// RoomBase Actor도 계산된 위치 근처로 이동 (디버깅 및 가시성을 위해)
		FVector RoomActorLocation = T_level.GetLocation();
		SetActorLocation(RoomActorLocation);
		SetActorRotation(T_level.GetRotation());

		UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - RoomBase Actor 위치 업데이트: %s"), *RoomActorLocation.ToString());

		// 앵커 캐시 갱신
		CacheAnchorsFromLevel();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ARoomBase::ApplyFinalLevelTransform - 레벨 재로드 실패"));
	}

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::ApplyFinalLevelTransform - 레벨 Transform 적용 및 가시화 완료"));
}

// DungeonGenerator에서 연결 정보를 설정하는 함수
void ARoomBase::SetConnectionTarget(const FTransform& TargetExitWorld)
{
	PendingTargetExitWorld = TargetExitWorld;
	bWaitingForLevelLoad = true;

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::SetConnectionTarget - 연결 대상 설정: %s"), *TargetExitWorld.ToString());

	// 이미 레벨이 로드되어 있다면 즉시 적용
	if (LoadedStreamingLevel && LoadedStreamingLevel->IsLevelLoaded())
	{
		ApplyFinalLevelTransform(TargetExitWorld);
		bWaitingForLevelLoad = false;
	}
}

// Temp 폴더의 액터들을 자동으로 삭제하는 함수 (최적화된 버전)
void ARoomBase::CleanupTempActors()
{
#if WITH_EDITOR

	// CleanupTempActors 비활성화 플래그 확인
	if (bDisableTempActorCleanup)
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::CleanupTempActors - Temp Actor Cleanup이 비활성화되어 있습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::CleanupTempActors - 함수 호출 시작 (재시도: %d/%d)"), CleanupRetryCount, MaxCleanupRetries);

	if (!LoadedStreamingLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::CleanupTempActors - LoadedStreamingLevel이 nullptr입니다."));
		CleanupRetryCount = 0; // 재시도 횟수 리셋
		return;
	}

	// 최대 재시도 횟수 초과 시 포기
	if (CleanupRetryCount >= MaxCleanupRetries)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::CleanupTempActors - 최대 재시도 횟수(%d)를 초과했습니다. 정리를 포기합니다."), MaxCleanupRetries);
		CleanupRetryCount = 0; // 재시도 횟수 리셋
		return;
	}

	// 레벨이 아직 로드 중인지 확인
	if (!LoadedStreamingLevel->IsLevelLoaded())
	{
		CleanupRetryCount++;
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::CleanupTempActors - 레벨이 아직 로드 중입니다. 잠시 후 다시 시도합니다. (%d/%d)"), CleanupRetryCount, MaxCleanupRetries);

		// 레벨 로드 완료를 기다리는 타이머 설정 (0.1초 후 재시도)
		GetWorld()->GetTimerManager().SetTimer(
			CleanupTimerHandle,
			this,
			&ARoomBase::CleanupTempActors,
			0.1f,
			false
		);
		return;
	}

	ULevel* LoadedLevel = LoadedStreamingLevel->GetLoadedLevel();
	if (!LoadedLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::CleanupTempActors - 로드된 레벨을 가져올 수 없습니다."));
		CleanupRetryCount = 0; // 재시도 횟수 리셋
		return;
	}

	int32 DeletedCount = 0;

	// 로드된 레벨의 모든 액터를 순회 (최적화: 역순으로 순회하여 삭제 안정성 향상)
	for (int32 i = LoadedLevel->Actors.Num() - 1; i >= 0; --i)
	{
		AActor* Actor = LoadedLevel->Actors[i];
		if (!Actor) continue;

		// 액터의 폴더 경로 확인 (성능 최적화: 문자열 비교 최소화)
		FString ActorFolderPath = Actor->GetFolderPath().ToString();

		// "Temp" 폴더에 있는 액터인지 확인 (대소문자 무시)
		if (ActorFolderPath.Equals(TEXT("Temp"), ESearchCase::IgnoreCase) ||
			ActorFolderPath.Contains(TEXT("Temp"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogTemp, Log, TEXT("ARoomBase::CleanupTempActors - Temp 폴더 액터 삭제: %s (폴더: %s)"),
				*Actor->GetName(), *ActorFolderPath);

			// 액터 삭제
			Actor->Destroy();
			DeletedCount++;
		}
	}

	if (DeletedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::CleanupTempActors - %d개의 Temp 폴더 액터가 삭제되었습니다."), DeletedCount);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::CleanupTempActors - 삭제할 Temp 폴더 액터가 없습니다."));
	}

	// 성공적으로 완료되면 재시도 횟수 리셋
	CleanupRetryCount = 0;
#endif
}

// 레벨 로드 시 Temp 폴더 액터 생성 방지
void ARoomBase::PreventTempActorCreation()
{
#if WITH_EDITOR
	// CleanupTempActors 비활성화 플래그 확인
	if (bDisableTempActorCleanup)
	{
		UE_LOG(LogTemp, Log, TEXT("ARoomBase::PreventTempActorCreation - Temp Actor Cleanup이 비활성화되어 있습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::PreventTempActorCreation - Temp 폴더 액터 생성 방지 시작"));

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::PreventTempActorCreation - World가 nullptr입니다."));
		return;
	}

	// 현재 레벨의 모든 액터를 확인하여 Temp 폴더에 있는 액터들을 즉시 삭제
	ULevel* CurrentLevel = World->GetCurrentLevel();
	if (CurrentLevel)
	{
		int32 DeletedCount = 0;

		// 역순으로 순회하여 안전하게 삭제
		for (int32 i = CurrentLevel->Actors.Num() - 1; i >= 0; --i)
		{
			AActor* Actor = CurrentLevel->Actors[i];
			if (!Actor) continue;

			FString ActorFolderPath = Actor->GetFolderPath().ToString();

			// Temp 폴더에 있는 액터인지 확인
			if (ActorFolderPath.Equals(TEXT("Temp"), ESearchCase::IgnoreCase) ||
				ActorFolderPath.Contains(TEXT("Temp"), ESearchCase::IgnoreCase))
			{
				UE_LOG(LogTemp, Log, TEXT("ARoomBase::PreventTempActorCreation - Temp 폴더 액터 즉시 삭제: %s"), *Actor->GetName());
				Actor->Destroy();
				DeletedCount++;
			}
		}

		if (DeletedCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("ARoomBase::PreventTempActorCreation - %d개의 Temp 폴더 액터가 즉시 삭제되었습니다."), DeletedCount);
		}
	}
#endif
}

// 메인 레벨 보호 함수
bool ARoomBase::IsMainLevel() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 현재 레벨이 메인 레벨인지 확인
	// 메인 레벨은 보통 Persistent Level이거나 특정 이름을 가짐
	ULevel* CurrentLevel = World->GetCurrentLevel();
	if (!CurrentLevel)
	{
		return false;
	}

	FString LevelName = CurrentLevel->GetName();

	// 메인 레벨 판별 조건들
	bool bIsMainLevel =
		LevelName.Contains(TEXT("Main")) ||
		LevelName.Contains(TEXT("Persistent")) ||
		LevelName.Contains(TEXT("Game")) ||
		LevelName.Contains(TEXT("DungeonKitchen")) ||
		!LevelName.Contains(TEXT("PLI_")); // Packed Level Instance가 아닌 경우

	UE_LOG(LogTemp, Log, TEXT("ARoomBase::IsMainLevel - 레벨 '%s'은 메인 레벨: %s"), *LevelName, bIsMainLevel ? TEXT("예") : TEXT("아니오"));

	return bIsMainLevel;
}

// CleanupTempActors 비활성화 설정 함수
void ARoomBase::SetDisableTempActorCleanup(bool bDisable)
{
	bDisableTempActorCleanup = bDisable;
	UE_LOG(LogTemp, Log, TEXT("ARoomBase::SetDisableTempActorCleanup - Temp Actor Cleanup %s"),
		bDisable ? TEXT("비활성화됨") : TEXT("활성화됨"));
}

// CleanupTempActors 비활성화 상태 확인 함수
bool ARoomBase::IsTempActorCleanupDisabled() const
{
	return bDisableTempActorCleanup;
}

