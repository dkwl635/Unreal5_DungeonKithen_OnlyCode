// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "RoomBase.generated.h"

// 레벨 스트리밍 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStreamingComplete, ARoomBase*, Room);

// Exit 위치 업데이트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExitLocationUpdated, ARoomBase*, Room, FTransform, ExitTransform);

/**
 * 모든 방 BP의 부모 클래스
 * Entrance/Exit 화살표 컴포넌트를 표준으로 제공
 * 바닥 피벗 없음, 틱 불필요
 */
UCLASS(BlueprintType, Blueprintable)
class DUNGEONKITCHEN_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// 생성자
	ARoomBase();

protected:
	// 게임 시작 시 호출
	virtual void BeginPlay() override;
	
	// 게임 종료 시 호출
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	// === Arrows ===
	// 입구 화살표 컴포넌트 (초록색, 게임에서 숨김)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrows")
	UArrowComponent* Entrance;

	// 출구 화살표 컴포넌트 (빨간색, 게임에서 숨김)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrows")
	UArrowComponent* Exit;

	// === Level Streaming 설정 ===
	// 에디터에서 설정할 Level 에셋 (스트리밍으로 로드됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming", meta = (DisplayName = "Room Level", AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> RoomLevel;
	
	// 레벨의 상대적 위치와 회전 (Room Actor 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming", meta = (DisplayName = "Level Transform"))
	FTransform LevelTransform;

	// 화살표 변환 정보를 가져오는 정적 함수 (단일 출구용 - 호환성 유지)
	UFUNCTION(BlueprintCallable, Category = "Room Utils")
	static bool GetArrowTransforms(ARoomBase* Room, FTransform& OutEntranceLocal, FTransform& OutExitWorld);



	// Room Level을 스트리밍으로 로드하는 함수 (비동기)
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void LoadRoomLevelAsync();
	
	// Room Level을 언로드하는 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void UnloadRoomLevel();
	
	// 레벨 로드 완료 시 호출되는 함수 (복잡한 버전)
	UFUNCTION()
	void OnRoomLevelLoaded();
	
	
	// 레벨에서 앵커를 찾는 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	AActor* FindAnchorInLevel(ULevel* Level, const FString& Tag);
	
	// 최종 레벨 Transform을 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void ApplyFinalLevelTransform(const FTransform& TargetExitWorld);
	
	// DungeonGenerator에서 연결 정보를 설정하는 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void SetConnectionTarget(const FTransform& TargetExitWorld);
	
	// Temp 폴더의 액터들을 자동으로 삭제하는 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void CleanupTempActors();
	
	// 앵커를 캐시하는 최적화된 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	bool CacheAnchorsFromLevel();
	
	// 레벨 로드 시 Temp 폴더 액터 생성 방지
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void PreventTempActorCreation();
	
	// 메인 레벨 보호 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	bool IsMainLevel() const;
	
	// CleanupTempActors 비활성화 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	void SetDisableTempActorCleanup(bool bDisable);
	
	// CleanupTempActors 비활성화 상태 확인 함수
	UFUNCTION(BlueprintCallable, Category = "Level Streaming")
	bool IsTempActorCleanupDisabled() const;
	
	// 레벨 스트리밍 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Level Streaming")
	FOnLevelStreamingComplete OnLevelStreamingComplete;
	
	// Exit 위치 업데이트 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Level Streaming")
	FOnExitLocationUpdated OnExitLocationUpdated;

protected:
	// 컴포넌트들을 설정하는 함수
	void SetupComponents();
	
	// 로드된 스트리밍 레벨을 추적하기 위한 변수
	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> LoadedStreamingLevel;
	
	// 로드 상태 체크 타이머 핸들 (더 이상 필요 없음)
	FTimerHandle LoadStatusTimerHandle;
	
	// Cleanup 재시도 타이머 핸들
	FTimerHandle CleanupTimerHandle;
	
	// 레벨 로드 대기 중인 Target Exit Transform
	UPROPERTY()
	FTransform PendingTargetExitWorld;
	
	// 로드 완료 대기 중인지 여부
	UPROPERTY()
	bool bWaitingForLevelLoad;
	
	// 캐시된 앵커 액터들 (성능 최적화)
	UPROPERTY()
	TObjectPtr<AActor> CachedEntranceActor;
	
	UPROPERTY()
	TObjectPtr<AActor> CachedExitActor;
	
	// 180도 회전 Transform (성능 최적화)
	static const FTransform R180Transform;
	
	// CleanupTempActors 재시도 횟수 제한
	UPROPERTY()
	int32 CleanupRetryCount;
	
	// 최대 재시도 횟수
	static const int32 MaxCleanupRetries = 10;
	
	// CleanupTempActors 비활성화 플래그 (에디터에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Streaming", meta = (DisplayName = "Disable Temp Actor Cleanup"))
	bool bDisableTempActorCleanup = false;
};
