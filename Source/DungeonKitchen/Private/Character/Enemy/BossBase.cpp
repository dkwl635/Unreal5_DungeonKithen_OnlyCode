// Copyright © 2025 Tartare Studio


#include "Character/Enemy/BossBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "GameplayEffect.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "AbilitySystem/DKAbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Character/Enemy/State/BossStateBase.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Game/Subsystem/DataManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/DKPlayWidget.h"
#include "DefaultLevelSequenceInstanceData.h"

int32 ABossBase::BossLevel = 1;

ABossBase::ABossBase()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<UDKAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UDKCharacterAttributeSet>(TEXT("AttributeSet"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UBossStateBase* ABossBase::GetState(EBossStateType State)
{
	return GetOrCreateState(State);
}

void ABossBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UDKAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//Enemy Data
	SetBossData();
	SetBossInit();

	// GameplayEffect 적용 감지를 위한 델리게이트 바인딩
	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ABossBase::OnGameplayEffectApplied);
	//SetTarget
	AActor* Find = UGameplayStatics::GetActorOfClass( Owner->GetWorld(), ADKPlayerCharacter::StaticClass());
	if (Find != nullptr)
	{
		Target = Cast<ADKPlayerCharacter>(Find);
	}

	StartPattern(EBossStateType::Idle);


	if (BossHealth_Class)
	{
		healthBar = CreateWidget<UDKPlayWidget>(GetWorld(), BossHealth_Class.Get());
		healthBar->AddToViewport();
		healthBar->SetWidgetController(this);
	}

	OnPlayLevelSequence();
}

void ABossBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInitialized) return;

	if (CurrentStateBase)
	{
		CurrentStateBase->Update(DeltaTime);
	}
}

void ABossBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ABossBase::OnDieEvent()
{
	OnBossDieEvent.Broadcast(this);

}

void ABossBase::OnPlayLevelSequence()
{
	if (!BossSequence)
		return;


	healthBar->RemoveFromParent();
	SetActorHiddenInGame(true);
	StartPattern(EBossStateType::Spawn);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this , &ThisClass::OnEndLevelSequence, 5.0f, false);

	// Level Sequence Player와 Actor를 생성
	ALevelSequenceActor* OutActor = nullptr;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		BossSequence,
		FMovieSceneSequencePlaybackSettings(),
		OutActor
	);

	if (SequencePlayer)
	{
		OutActor->bOverrideInstanceData = true;
		// InstanceData를 ULevelSequenceDefaultData 타입으로 캐스팅합니다.
		// ULevelSequenceDefaultData는 TransformOrigin Actor와 같은 속성을 가지고 있습니다.
		if (UDefaultLevelSequenceInstanceData* DefaultData = Cast<UDefaultLevelSequenceInstanceData>(OutActor->DefaultInstanceData.Get()))
		{
			// 원하는 액터 (예: 현재 보스 액터 'this' 또는 레벨의 다른 액터)를 원점으로 설정
			// 이 보스가 움직이면 시퀀스 전체가 따라 움직이게 됩니다.
			DefaultData->TransformOriginActor = this;

			// 만약 TransformOrigin을 월드 좌표로 직접 지정하고 싶다면:
			// DefaultData->TransformOrigin = FVector(100.0f, 0.0f, 0.0f);
		}
		SequencePlayer->Play();
	}

}

void ABossBase::OnEndLevelSequence()
{
	healthBar->AddToViewport();
	SetActorHiddenInGame(false);
	StartPattern(EBossStateType::Idle);
}

void ABossBase::OnPatternCompleted(EBossStateType CompletedPattern)
{
	UE_LOG(LogTemp, Display, TEXT("Pattern Completed: %d"), (int32)CompletedPattern);

	if (CompletedPattern == EBossStateType::Death)
		return;

	EBossStateType NewState = SelectPatternByWeight(CompletedPattern);

	StartPattern(NewState);
}

EBossStateType ABossBase::SelectPatternByWeight(EBossStateType PreviousPattern)
{
	float TotalWeight = 0.0f;
	TArray<Chaos::Pair<EBossStateType, float> > States;

	for (auto& State : StateMap)
	{
		EBossStateType CurrentStateType = State.Key;
		float weight = State.Value->CalculatePatternWeight(PreviousPattern);

		if (weight <= 0.0f)
			continue;
		States.Add({CurrentStateType, weight});
		TotalWeight += weight;
	}


	// 랜덤 값 생성 (0 ~ TotalWeight)
	float RandomValue = FMath::RandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	// 가중치에 따라 패턴 선택
	for (const auto& WeightPair : States)
	{
		EBossStateType PatternType = WeightPair.First;
		float BaseWeight = WeightPair.Second;

		CurrentWeight += BaseWeight;
		if (RandomValue <= CurrentWeight)
		{
			return PatternType;
		}
	}

	return  EBossStateType::Idle;
}

void ABossBase::StartPattern(EBossStateType NextPattern)
{
	if (NextPattern == EBossStateType::NONE)
		return;
	if (CurrentState == EBossStateType::Death)
		return;

	CurrentState = NextPattern;

	// 공격 패턴인 경우 카운트 증가
	if (NextPattern == EBossStateType::NormalAttack1 ||
		NextPattern == EBossStateType::NormalAttack2 ||
		NextPattern == EBossStateType::NormalAttack3 ||
		NextPattern == EBossStateType::SpecialAttack1 ||
		NextPattern == EBossStateType::SpecialAttack2 ||
		NextPattern == EBossStateType::SpecialAttack3)
	{
		AttackCount++;
	}
	// Idle 상태로 돌아갈 때 공격 카운트 리셋
	else if (NextPattern == EBossStateType::Idle)
	{
		ResetAttackCount();
	}

	UBossStateBase* NewBaseState = GetOrCreateState(CurrentState);

	if (IsValid(NewBaseState))
	{
		NewBaseState->StartState();
		CurrentStateBase = NewBaseState;
	}
	else
	{
		CurrentStateBase = nullptr;
	}
}

void ABossBase::ResetAttackCount()
{
	AttackCount = 0;
}

void ABossBase::EnableLookAtTarget()
{
	if (AIController && Target)
	{
		// AI 컨트롤러를 통해 타겟을 바라보도록 설정
		// 실제 AI 시스템에 따라 구현이 달라질 수 있습니다
		// 예시: AIController->SetFocus(Target);
		GetAIController()->SetFocus(Target);

		UE_LOG(LogTemp, Log, TEXT("EnableLookAtTarget - Boss is now looking at target"));
	}
}

void ABossBase::DisableLookAtTarget()
{
	if (AIController)
	{
		// AI 컨트롤러의 포커스를 해제
		// 예시: AIController->ClearFocus(EAIFocusPriority::Gameplay);
		GetAIController()->ClearFocus(EAIFocusPriority::Gameplay);
		UE_LOG(LogTemp, Log, TEXT("DisableLookAtTarget - Boss stopped looking at target"));

	}
}

void ABossBase::OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	// 바라보기 종료
	if (NotifyName == FName("StopLookAt"))
	{
		DisableLookAtTarget();
		UE_LOG(LogTemp, Log, TEXT("Animation Notify: %s - Disabled LookAt"), *NotifyName.ToString());
	}
	// 바라보기 시작
	else if (NotifyName == FName("StartLookAt"))
	{
		EnableLookAtTarget();
		UE_LOG(LogTemp, Log, TEXT("Animation Notify: %s - Enabled LookAt"), *NotifyName.ToString());
	}

	if (CurrentStateBase)
	{
		CurrentStateBase->OnAnimNotifyBegin(NotifyName, BranchingPointPayload);
	}
}

float ABossBase::GetTargetDistance()
{
	if (Target)
	{
		return GetDistanceTo(Target);
	}
	return 0.0f;
}

void ABossBase::ApplyDamageToTarget(TSubclassOf<UGameplayEffect> EffectClass, AActor* ToTaget)
{
	if (!EffectClass || !ToTaget) return;


	// 플레이어의 Ability System Component 가져오기
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ToTaget);
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponent();
	if (!TargetASC)
		return;

	//효과의 컨텍스트 생성
	FGameplayEffectContextHandle EffectContextHandle = 	OwnerASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	//GameplayEffect spec 생성
	const FGameplayEffectSpecHandle EffectSpecHandle = OwnerASC->MakeOutgoingSpec(EffectClass,Level, EffectContextHandle);
	//생성한 이펙트 사양을 대상에게 직접 적용
	const FActiveGameplayEffectHandle ActiveEffectHandle = OwnerASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetASC);

}



void ABossBase::SetBossData()
{
	if (UDataManager* DataManager = GetGameInstance()->GetSubsystem<UDataManager>())
	{
		if (!DataManager->GetEnemyData(BossID, EnemyData ))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed Load EnemyData"));
			return;
		}

		float MaxHp = EnemyData.HP * (BossLevel ) ;
		float Atk =   EnemyData.Damage * (BossLevel) ;
		BossName = EnemyData.EnemyName;

		//값 셋팅
		UDKCharacterAttributeSet* DKAttribute = Cast<UDKCharacterAttributeSet> (GetAttributeSet());
		DKAttribute->InitMaxHP (MaxHp);
		DKAttribute->InitHP(MaxHp);
		DKAttribute->InitATK(Atk);
		//InitializePrimaryAttributes();
	}

}

void ABossBase::SetBossInit()
{
	bIsInitialized = true;

	AIController = Cast<AAIController>(GetController());
	AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->OnMontageEnded.AddDynamic(this, &ABossBase::OnMontageEnded);
	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABossBase::OnAnimNotifyBegin);

	GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);

	if (GetAbilitySystemComponent())
	{
		UDKCharacterAttributeSet* DKAttribute = Cast<UDKCharacterAttributeSet> (GetAttributeSet());
		GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(DKAttribute->GetHPAttribute()).AddUObject(this, &ABossBase::OnHPChanged);
	}

	for (auto& data : StateClassMap )
	{
		GetOrCreateState(data.Key);
	}




}

UBossStateBase* ABossBase::GetOrCreateState(EBossStateType BossState)
{
	if (StateMap.Contains(BossState) && StateMap[BossState])
	{
		return StateMap[BossState];
	}

	if (!StateClassMap.Contains(BossState) || !StateClassMap[BossState])
		return nullptr;

	UBossStateBase* NewState = NewObject<UBossStateBase>(this, StateClassMap[BossState]);
	if (NewState)
	{
		NewState->SetOwner(this);
		StateMap.Add(BossState, NewState);
	}


	return NewState;
}

bool ABossBase::IsHaveState(EBossStateType BossState)
{
	if (!StateClassMap.Contains(BossState) || !StateClassMap[BossState])
		return false;

	return true;
}

void ABossBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	CurrentStateBase->OnMontageEnded(Montage, bInterrupted);
}

void ABossBase::OnGameplayEffectApplied(UAbilitySystemComponent* TargetAbility, const FGameplayEffectSpec& SpecApplied,
	FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer GameplayTags;
	SpecApplied.GetAllAssetTags(GameplayTags);

	// 적용된 GameplayEffect에 Attack 태그가 있는지 확인
	if (GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Attack"))))
	{
		// Attack 태그가 감지되면 Hit 상태로 변경
		//ChangeState(EEnemyStateType::Hit);
	}
}

void ABossBase::OnHPChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp , Warning , TEXT("Boss : HP : %f") , Data.NewValue );

	float Hp = FMath::Max(Data.NewValue ,0);
	OnChangeBossHp.Broadcast(Hp);

	if (Data.NewValue <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossDeath"));
		StartPattern(EBossStateType::Death);

	}

}
