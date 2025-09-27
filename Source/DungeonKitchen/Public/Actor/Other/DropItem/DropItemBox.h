// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DropItemBox.generated.h"

UENUM()
enum class EDropItemState : uint8 { Ballistic, Waiting, Homing, Collected };

UCLASS()
class DUNGEONKITCHEN_API ADropItemBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADropItemBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USphereComponent> RootCoomp;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;
	// 플레이어 근접 감지를 위한 오버랩 전용 콜라이더
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USphereComponent> ProximitySphere;

    // 바닥에 떨어졌을 때 표시할 이펙트(예: 반짝이는 나이아가라 효과)
    UPROPERTY(EditAnywhere)
    TObjectPtr<class UNiagaraComponent> GroundVfx;

	UPROPERTY()
	class UDKFood* Food;

public:
	void SetItemBox(int32 InItemID  , int32 InCount);
	UFUNCTION(BlueprintCallable)
	void GetItem();

	// 플레이어가 근접했을 때 타겟을 설정하여 흡입 시작
	UFUNCTION()
	void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 내부 유틸: 지정 캐릭터를 향해 호밍 시작
    void StartHomingTo(class ACharacter* PC);

    // 틱에서 근접 오버랩 상태를 폴링하여 스폰 직후 겹침도 대응
    void TryStartHomingIfOverlapping();

protected:

	UPROPERTY(EditAnywhere)
	int32 ItemID = -1;
	UPROPERTY(EditAnywhere)
	int32 Count = -1;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> Target;

	UPROPERTY(EditAnywhere)
	float Speed = 400.0f;
	UPROPERTY(EditAnywhere)
	float InitSpeed = 400.0f;

    // --- Movement parameters ---
    UPROPERTY(EditAnywhere)
    float GravityZ = -980.f;
    UPROPERTY(EditAnywhere)
    float InitialUpSpeed = 600.f;
    UPROPERTY(EditAnywhere)
    float InitialOutSpeed = 300.f;
    UPROPERTY(EditAnywhere)
    float HomingMaxSpeed = 1600.f;
    UPROPERTY(EditAnywhere)
    float HomingAccel = 3000.f;
    UPROPERTY(EditAnywhere)
    float SpinSpeed = 180.f; // deg/sec, waiting 상태에서 회전 속도

    // 도착 브레이킹 및 호밍 안정화 파라미터
    UPROPERTY(EditAnywhere)
    float ArrivalRadius = 150.f;    // 이 거리부터 점차 감속
    UPROPERTY(EditAnywhere)
    float StopRadius = 60.f;        // 이 이내에서는 사실상 0속도 목표
    UPROPERTY(EditAnywhere)
    float HomingDamping = 8.f;      // (desiredVel - vel) 게인. 높을수록 빠르게 정렬, 너무 높으면 떨림

	FVector StartPos ;
    FVector Velocity;
    EDropItemState State = EDropItemState::Ballistic;

	bool bIsGet = false;
};
