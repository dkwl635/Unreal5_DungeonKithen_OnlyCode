// Copyright © 2025 Tartare Studio


#include "Actor/Other/DropItem/DropItemBox.h"
#include "Components/SphereComponent.h"
#include "Game/Subsystem/DataManager.h"
#include "Game/Subsystem/PlayerInventoryManager.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

class UDataManager;
// Sets default values
ADropItemBox::ADropItemBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    RootCoomp = CreateDefaultSubobject<USphereComponent>(TEXT("RootComp"));
	SetRootComponent(RootCoomp);
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootCoomp);

    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 근접 오버랩용 스피어
    ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
    ProximitySphere->SetupAttachment(RootCoomp);
    ProximitySphere->InitSphereRadius(200.f);
    ProximitySphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProximitySphere->SetCollisionResponseToAllChannels(ECR_Overlap);

    // 바닥 도착 시 표시할 이펙트(기본 비활성)
    GroundVfx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("GroundVfx"));
    GroundVfx->SetupAttachment(RootCoomp);
    GroundVfx->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ADropItemBox::BeginPlay()
{
	Super::BeginPlay();
    if (ProximitySphere)
    {
        ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &ADropItemBox::OnProximityBeginOverlap);
    }
    // Root 오버랩 획득 처리는 블루프린트에서 수행
}

// Called every frame
void ADropItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    switch (State)
    {
    case EDropItemState::Ballistic:
    {
        // 중력 적용 포물선 이동
        Velocity += FVector(0,0,GravityZ) * DeltaTime;
        FHitResult Hit;
        SetActorLocation(GetActorLocation() + Velocity * DeltaTime, true, &Hit);
        // 스폰 직후 이미 플레이어와 겹친 경우 즉시 호밍 전환
        TryStartHomingIfOverlapping();
        if (Hit.bBlockingHit)
        {
            Velocity = FVector::ZeroVector;
            State = EDropItemState::Waiting;
            if (GroundVfx)
            {
                GroundVfx->Activate(true);
            }
        }
        break;
    }
    case EDropItemState::Waiting:
    {
        // 대기 중에는 빙글빙글 회전
        AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));
        // 플레이어가 이미 프로ximity에 겹쳐있는 경우 대응
        TryStartHomingIfOverlapping();
        break;
    }
    case EDropItemState::Homing:
    {
        if (!Target) { State = EDropItemState::Waiting; break; }
        const FVector ToTarget = (Target->GetComponentLocation() - GetActorLocation());
        const float Dist = ToTarget.Size();
        const FVector Dir = (Dist > KINDA_SMALL_NUMBER) ? (ToTarget / Dist) : FVector::ZeroVector;

        // 도착 반경 감속: 목표 속도 크기를 거리 기반으로 줄임
        float desiredSpeed = HomingMaxSpeed;
        if (Dist < ArrivalRadius)
        {
            desiredSpeed = FMath::GetMappedRangeValueClamped(FVector2D(StopRadius, ArrivalRadius), FVector2D(0.f, HomingMaxSpeed), Dist);
        }
        const FVector DesiredVel = Dir * desiredSpeed;

        // 크리티컬 감쇠 형태의 속도 보정(스티어링): vel += (desiredVel - vel) * k * dt
        Velocity += (DesiredVel - Velocity) * HomingDamping * DeltaTime;
        Velocity = Velocity.GetClampedToMaxSize(HomingMaxSpeed);
        FHitResult Hit;
        SetActorLocation(GetActorLocation() + Velocity * DeltaTime, true, &Hit);
        if (Dist <= StopRadius)
        {
            Velocity = FVector::ZeroVector;
        }
        break;
    }
    case EDropItemState::Collected:
    default:
        break;
    }
}

void ADropItemBox::SetItemBox(int32 InItemID, int32 InCount)
{
	bIsGet = false;

	ItemID = InItemID;
	Count = InCount;

    // 초기 포물선: 위 + 랜덤 수평 방향
    const FVector Up = FVector::UpVector * InitialUpSpeed;
    const FVector Out = FRotationMatrix(FRotator(0, FMath::RandRange(0.f, 360.f), 0)).GetUnitAxis(EAxis::X) * InitialOutSpeed;
    Velocity = Up + Out;
    State = EDropItemState::Ballistic;
    Target = nullptr;
    StartPos = GetActorLocation();
    Speed = InitSpeed;

	UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>();
	if (!DataManager)
	{
		UE_LOG(LogTemp, Warning , TEXT("PlayerInventoryManager::AddItem - GetGameInstance is null"));
		return;
	}

	Food = DataManager->GetFoodItem(InItemID, 1);
	if (Food)
	{
		MeshComp->SetStaticMesh(Food->GetMesh());
	}

}

void ADropItemBox::GetItem()
{
	if (bIsGet) return;
	bIsGet = true;

	FString str =  FString::Printf(TEXT("Get Item ID : %d , Count : %d"), ItemID, Count);
	GEngine->AddOnScreenDebugMessage(0,1.0f, FColor::Green , str);

	// 비활성화
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	Target = nullptr;

	//GetGameInstance()->GetSubsystem<UPlayerInventoryManager>()->AddFood(ItemID);
	GetGameInstance()->GetSubsystem<UPlayerInventoryManager>()->FirstAddFood(Food);

}

void ADropItemBox::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsGet) return;

    if (ACharacter* PC = Cast<ACharacter>(OtherActor))
    {
        StartHomingTo(PC);
    }
}

void ADropItemBox::TryStartHomingIfOverlapping()
{
    if (State == EDropItemState::Homing || bIsGet) return;
    if (!ProximitySphere) return;

    // 플레이어가 스폰 순간부터 근접 영역 안에 있었던 케이스 대응
    ACharacter* PC = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!PC) return;

    if (ProximitySphere->IsOverlappingActor(PC))
    {
        StartHomingTo(PC);
    }
}

void ADropItemBox::StartHomingTo(ACharacter* PC)
{
    if (!PC) return;
    Target = PC->GetMesh();
    State = EDropItemState::Homing;
    if (RootCoomp)
    {
        RootCoomp->SetCollisionProfileName(TEXT("PlayerTrigger"));
    }
    if (GroundVfx)
    {
        GroundVfx->Deactivate();
    }
    const FVector Dir = (Target->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
    const float InitialHomingSpeed = FMath::Max(InitSpeed, HomingMaxSpeed * 0.25f);
    Velocity = Dir * InitialHomingSpeed;
    
    // Ballistic 상태에서 즉시 호밍으로 전환된 경우, 중력을 무시하고 호밍 시작
    if (State == EDropItemState::Homing)
    {
        // 중력 제거하고 순수 호밍 속도로 설정
        Velocity.Z = 0.f; // 중력 제거
        Velocity = Dir * InitialHomingSpeed;
    }
}

// Root 오버랩 획득은 블루프린트에서 처리합니다.

