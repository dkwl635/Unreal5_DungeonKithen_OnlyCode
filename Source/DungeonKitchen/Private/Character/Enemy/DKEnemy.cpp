// Copyright © 2025 Tartare Studio


#include "Character/Enemy/DKEnemy.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "DKGameplayTags.h"
#include "NavigationSystem.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "AbilitySystem/DKAbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Actor/Enemy/EnemyAttacker.h"
#include "Character/Enemy/DKEnemyAttack.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Game/Subsystem/DataManager.h"
#include "Game/Subsystem/DigestiveSystem.h"
#include "Game/Subsystem/PoolWorldSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "UI/Widgets/DKPlayWidget.h"

int32 ADKEnemy::EnemyLevel = 1;

ADKEnemy::ADKEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));

	AbilitySystemComponent = CreateDefaultSubobject<UDKAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UDKCharacterAttributeSet>(TEXT("AttributeSet"));

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 위젯 블루프린트 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> HealthBarWidgetClass(TEXT("/Game/02_Enemy/Widget/WBP_DKEnemyHealthBar"));
	if (HealthBarWidgetClass.Succeeded())
	{
		HealthBar->SetWidgetClass(HealthBarWidgetClass.Class);
		HealthBar->SetDrawSize(FVector2D(500.0f, 500.0f));
		HealthBar->SetCastShadow(false);
	}

	///Script/Engine.BlueprintGeneratedClass'/Game/02_Enemy/GameplayEffect/GE_Enemy_Attack.GE_Enemy_Attack_C'
	static ConstructorHelpers::FClassFinder<UGameplayEffect> AttackEffectClass(TEXT("/Game/02_Enemy/GameplayEffect/GE_Enemy_Attack"));
	if (AttackEffectClass.Succeeded())
	{
		DamageGameplayEffect = AttackEffectClass.Class;
	}


	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bIsAlive = true;
	bIsHit = false;

	// Enemy Sprite
	static ConstructorHelpers::FObjectFinder<UPaperSprite> EnemySprite(TEXT("/Script/Paper2D.PaperSprite'/Game/KHH/UI/MiniMap/MonsterSprite.MonsterSprite'"));
	if (EnemySprite.Succeeded())
	{
		MinimapSprite->SetSprite(EnemySprite.Object);
		MinimapSprite->SetSpriteColor(FColor::Red);
	}
}

void ADKEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitEnemyData();
	EnemySetting();
	StartState(EDKEnemyState::Idle);
}

void ADKEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHealthBarBillboard();
	FindTarget();
	switch (CurrentState)
	{
	case EDKEnemyState::Trace:
		TickStateTrace(DeltaTime);
		break;
		default:
		break;
	}
}

void ADKEnemy::InitEnemyData()
{
	if (bIsInitialized)
		return;
	bIsInitialized = true;

	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (!DataManager->GetEnemyData(EnemyID, EnemyData ))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed Load EnemyData"));
			return;
		}

		float MaxHp = EnemyData.HP * EnemyLevel ;
		float Atk =  EnemyData.Damage * EnemyLevel ;
		float Def = 0.0f * EnemyLevel;

		EnemyName = EnemyData.EnemyName;

		//값 셋팅
		UDKCharacterAttributeSet* DKAttribute = Cast<UDKCharacterAttributeSet> (GetAttributeSet());
		DKAttribute->InitMaxHP (MaxHp);
		DKAttribute->InitHP(MaxHp);
		DKAttribute->InitATK(Atk);
		DKAttribute->InitDEF(Def);

	}

}

void ADKEnemy::EnemySetting()
{
	AIController = Cast<AAIController>(GetController());
	AnimInstance = GetMesh()->GetAnimInstance();
	//몽타주 리턴
	AnimInstance->OnMontageEnded.AddDynamic(this, &ADKEnemy::OnMontageEnded);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ADKEnemy::OnAnimNotifyBegin);

	GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);

	if (GetAbilitySystemComponent())
	{
		UDKCharacterAttributeSet* DKAttribute = Cast<UDKCharacterAttributeSet> (GetAttributeSet());
		GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(DKAttribute->GetHPAttribute()).AddUObject(this, &ADKEnemy::OnHPChanged);
		GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(DKAttribute->GetMaxHPAttribute()).AddUObject(this, &ADKEnemy::OnMaxHPChanged);
		// GameplayEffect 적용 감지를 위한 델리게이트 바인딩
		AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ADKEnemy::OnGameplayEffectApplied);

	}

	// HP바 기본 설정
	if (HealthBar)
	{
		HealthBar->SetVisibility(true);
		HealthBar->SetUsingAbsoluteRotation(true);  // 절대 회전 사용

		// 월드 공간에서 렌더링되도록 설정
		HealthBar->SetWidgetSpace(EWidgetSpace::World);

		//Set Hpbar
		UDKPlayWidget* HpWidget = Cast<UDKPlayWidget>(HealthBar->GetWidget());
		HpWidget->SetWidgetController(this);
	}



	if (EnemyAttackClass)
	{
		EnemyAttack = NewObject<UDKEnemyAttack>(this, EnemyAttackClass);
	}

	bIsAlive = true;
	bIsHit = false;
}

 AEnemyAttacker* ADKEnemy::CreateEnemyAttacker(TSubclassOf<AEnemyAttacker> EnemyAttackerClass, FVector Location,
	FRotator Rotation)
{
	FTransform Transform;
	Transform.SetLocation(Location);
	Transform.SetRotation(Rotation.Quaternion());
	AEnemyAttacker* EnemyAttacker = GetWorld()->SpawnActorDeferred<AEnemyAttacker>(EnemyAttackerClass, Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!EnemyAttacker)
	{
		UE_LOG(LogTemp , Warning, TEXT("Enemy Spawn Failed"));
		return nullptr;
	}

	EnemyAttacker->SetEnemy(this);

	UGameplayStatics::FinishSpawningActor(EnemyAttacker, Transform);
	return EnemyAttacker;
}


void ADKEnemy::ApplyDamageToTarget(AActor* DamageTarget)
{
	if (!DamageTarget)
		return;

	// 플레이어의 Ability System Component 가져오기
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(DamageTarget);
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponent();
	if (!TargetASC)
		return;


	// 데미지 적용 (간단한 예시)
	if (TargetASC == nullptr) return;
	check(DamageGameplayEffect)

	//효과의 컨텍스트 생성
	FGameplayEffectContextHandle EffectContextHandle = 	OwnerASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(Owner);
	//GameplayEffect spec 생성
	const FGameplayEffectSpecHandle EffectSpecHandle = OwnerASC->MakeOutgoingSpec(DamageGameplayEffect, Level, EffectContextHandle);
	FDKGameplayTags GameplayTags = FDKGameplayTags::Get();
	UDKCharacterAttributeSet* Att = Cast<UDKCharacterAttributeSet>(GetAttributeSet());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Damage, Att->GetATK());

	//생성한 이펙트 사양을 대상에게 직접 적용
	const FActiveGameplayEffectHandle ActiveEffectHandle = OwnerASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);
}


void ADKEnemy::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		//중간에 다른 문제 발생시
	}
	else
	{
		switch (CurrentState)
		{
			case EDKEnemyState::Attack :
				StartState(EDKEnemyState::Idle);
				break;
			case EDKEnemyState::Death :
			// 애니메이션 정지
			AnimInstance->StopAllMontages(0.0f);
			SetLifeSpan(2.0f);
			break;
			case EDKEnemyState::Hit :
			bIsHit = false;
			StartState(EDKEnemyState::Trace);
			break;
			default:
				break;
		}
	}
}

void ADKEnemy::OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == TEXT("Attack"))
	{


		if (EnemyAttack)
		{
			EnemyAttack->StartAttack(this);
		}
	}

}

void ADKEnemy::OnGameplayEffectApplied(UAbilitySystemComponent* TargetAbility, const FGameplayEffectSpec& SpecApplied,
	FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer GameplayTags;
	SpecApplied.GetAllAssetTags(GameplayTags);

	// 적용된 GameplayEffect에 Attack 태그가 있는지 확인
	if (GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Attack"))))
	{
		if (bIsHit || !bIsAlive)
		{
			return;
		}
		// Attack 태그가 감지되면 Hit 상태로 변경
		StartState(EDKEnemyState::Hit);
	}

}

void ADKEnemy::OnHPChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp , Warning , TEXT("HP : %f") , Data.NewValue );

	if (Data.NewValue <= 0.0f)
	{
		StartState(EDKEnemyState::Death);
	}

	OnChangeHP.Broadcast(Data.NewValue);
}

void ADKEnemy::OnMaxHPChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp , Warning , TEXT("MaxHP : %f") , Data.NewValue );
	OnChangeMaxHP.Broadcast(Data.NewValue);
}

void ADKEnemy::StartState(EDKEnemyState NewState)
{
	if (CurrentState == NewState)
		return;

	if (CurrentState == EDKEnemyState::Death)
		return;

	CurrentState = NewState;
	switch (NewState)
	{
	case EDKEnemyState::Idle :
		StartStateIdle();
		break;
	case EDKEnemyState::Trace:
		StartStateTrace();
		break;
	case EDKEnemyState::Attack :
		StartStateAttack();
		break;
	case EDKEnemyState::Hit:
		StartStateHit();
		break;
	case EDKEnemyState::Death:
		StartStateDeath();
		break;
	default:
		break;

	}
}

void ADKEnemy::FindTarget()
{
	if (!Target.IsValid())
	{
		AActor* Find = UGameplayStatics::GetActorOfClass( Owner->GetWorld(), ADKPlayerCharacter::StaticClass());
		if (Find != nullptr)
		{
			Target = Cast<ADKPlayerCharacter>(Find);

		}
	}

}

void ADKEnemy::StartStateIdle()
{
	bIsHit = false;
	FindTarget();

	AIController->StopMovement();
	if (GetTarget())
	{
		StartState(EDKEnemyState::Trace);
	}
}



void ADKEnemy::StartStateTrace()
{
	bIsHit = false;
	if (AIController && GetTarget())
	{
		AIController->MoveToActor(GetTarget());
	}
}

void ADKEnemy::StartStateAttack()
{
	bIsHit = true;
	AIController->StopMovement();
	AIController->SetFocus(GetTarget());
	AnimInstance->Montage_Play(StateMontage[EDKEnemyState::Attack]);
}

void ADKEnemy::StartStateHit()
{
	AIController->StopMovement();
	AnimInstance->Montage_Play(StateMontage[EDKEnemyState::Hit]);
	bIsHit = true;

	// 뒤로 밀리기 효과
	FVector HitDirection = -GetActorForwardVector();
	GetCharacterMovement()->Launch(HitDirection * 1000.0f);
}

void ADKEnemy::StartStateDeath()
{
	bIsAlive = false	;
	GetCharacterMovement()->Velocity = FVector(0, 0, 0);
	AIController->StopMovement();
	AIController->SetFocus(nullptr);
	AnimInstance->Montage_Play(StateMontage[EDKEnemyState::Death]);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBar->SetVisibility(false);

	//아이템 스폰
	int32 ItemId = GetSpawnItemID();
	if (ItemId != -1)
	{
		UPoolWorldSubsystem* Pool =	GetWorld()->GetSubsystem<UPoolWorldSubsystem>();
		if (Pool)
		{
			Pool->SpawnDropItemBox(GetMesh()->GetComponentLocation(), ItemId, 1);
		}
	}

	/*음식 효과체크 하기*/
	if (UDigestiveSystem* DigestiveSystem = GetWorld()->GetGameInstance()->GetSubsystem<UDigestiveSystem>())
	{
		DigestiveSystem->StartTriggerDishes(TEXT("OnKillCount") , this);
		DigestiveSystem->StartTriggerDishes(TEXT("OnKillRate") , this);
	}
	/*음식 효과체크 하기*/

	//외부 바인딩 된 이벤트
	OnEnemyDieEvent.Broadcast(this);
}

void ADKEnemy::TickStateTrace(float DeltaTime)
{
	if (!GetTarget())
		return;

	//길찾기 갱신
	LastMovementUpdateTime-= DeltaTime;
	if (LastMovementUpdateTime < 0.0f)
	{
		UpdatePathFollowing();
		LastMovementUpdateTime = MovementUpdateInterval;
	}



	float temp = GetDistanceTo(GetTarget());
	if (temp <= AttackDis)
	{
		StartState(EDKEnemyState::Attack);
	}
}

void ADKEnemy::UpdatePathFollowing()
{
	// AI 이동 설정 개선
	FAIMoveRequest MoveRequest;
	MoveRequest.SetAcceptanceRadius(50.0f);  // 목표 지점 근처에서 정지
	MoveRequest.SetUsePathfinding(true);  // 경로 찾기 사용
	//MoveRequest.SetAllowPartialPath(true);  // 부분 경로 허용

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(World);

	if (NavSystem)
	{
		FVector TargetLocation = GetTarget()->GetActorLocation();
		FNavLocation ValidLocation;

		// 타겟 위치를 NavMesh 위로 투영 (반경 500.0f 안에서 탐색)
		if (NavSystem->ProjectPointToNavigation(TargetLocation, ValidLocation, FVector(500.f, 500.f, 200.f)))
		{
			// 유효한 위치를 찾았으면 해당 위치로 이동
			MoveRequest.SetGoalLocation(ValidLocation.Location);
		}
		else
		{
			MoveRequest.SetGoalLocation(TargetLocation);
		}
	}

	MoveRequest.SetCanStrafe(true);  // 측면 이동 허용


	// AI 이동 실행
	FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);
	if (Result == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy %s failed to move to target , %f ,%f,%f"), *Owner->GetName(), MoveRequest.GetGoalLocation().X , MoveRequest.GetGoalLocation().Y, MoveRequest.GetGoalLocation().Z);
	}
}

void ADKEnemy::UpdateHealthBarBillboard()
{
	if (!HealthBar || !HealthBar->IsVisible())
	return;

	// 플레이어 카메라 찾기
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
		return;

	// 플레이어 카메라의 위치와 방향 계산
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// HealthBar를 월드 포지션으로 설정 (캐릭터 머리 위)
	FVector HealthBarLocation = HealthBar->GetComponentLocation();


	// 빌보드: 항상 카메라를 향하도록 회전
	FVector DirectionToCamera = (CameraLocation - HealthBarLocation).GetSafeNormal();
	FRotator BillboardRotation = DirectionToCamera.Rotation();


	HealthBar->SetWorldRotation(BillboardRotation);
}

int32 ADKEnemy::GetSpawnItemID()
{
	FDKEnemyData& Data = EnemyData;
	int32 totalRate = 0;
	for (int32 value : Data.DropItemRate)
	{
		totalRate += value;
	}

	int32 rand = FMath::RandRange(0, totalRate - 1);

	// 랜덤 값에 따라 해당하는 아이템 ID 찾기
	int32 currentRate = 0;
	for (int32 i = 0; i < Data.DropItemRate.Num(); i++)
	{
		currentRate += Data.DropItemRate[i];
		if (rand < currentRate)
		{
			return Data.DropItemID[i];
		}
	}

	return -1;
}
