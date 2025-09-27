// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Character/DKCharacterBase.h"
#include "Data/EnemyDataStructures.h"
#include "GameplayEffectTypes.h"
#include "BossBase.generated.h"

/**
 *
 */



UENUM(BlueprintType)
enum class EBossStateType : uint8
{
	Spawn UMETA(DisplayName = "Spawn"),

	Idle  UMETA(DisplayName = "Idle"),

	Trace  UMETA(DisplayName = "Trace"),

	NormalAttack1 UMETA(DisplayName = "NormalAttack1"),
	NormalAttack2 UMETA(DisplayName = "NormalAttack2"),
	NormalAttack3 UMETA(DisplayName = "NormalAttack3"),

	SpecialAttack1 UMETA(DisplayName = "SpecialAttack1"),
	SpecialAttack2 UMETA(DisplayName = "SpecialAttack2"),
	SpecialAttack3 UMETA(DisplayName = "SpecialAttack3"),


	Hit   UMETA(DisplayName = "Hit"),
	Death  UMETA(DisplayName = "Death"),


	NONE UMETA(DisplayName = "None"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossEvent, AActor* , Boss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossHpChanged, float ,Hp);

UCLASS()
class DUNGEONKITCHEN_API ABossBase : public ADKCharacterBase
{
	GENERATED_BODY()
public:
	static int32 BossLevel;
	UFUNCTION(BlueprintCallable)
	static void BossLevelUp(){BossLevel++;}
public:
	ABossBase();

	UPROPERTY(EditDefaultsOnly , Category = "Boss|State")
	TMap<EBossStateType, TSubclassOf<class UBossStateBase>> StateClassMap;

	UFUNCTION(BlueprintCallable)
	class AAIController* GetAIController(){return  AIController;}
	UFUNCTION(BlueprintCallable)
	UAnimInstance* GetAnimInstance(){return AnimInstance;}
	UFUNCTION(BlueprintCallable)
	ADKCharacterBase* GetTarget(){return Target;}
	UFUNCTION(BlueprintCallable)
	UBossStateBase* GetState(EBossStateType State);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EBossStateType CurrentState = EBossStateType::NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	int32 AttackCount = 0;



public:
	UPROPERTY(BlueprintAssignable)
	FOnBossEvent OnBossDieEvent;
	UPROPERTY(BlueprintAssignable)
	FOnBossHpChanged OnChangeBossHp;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsbAlive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spawn")
	int32 BossID = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Name")
	FName BossName = TEXT("BossName");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Level")
	float Level = 1.0f;

	FDKEnemyData EnemyData;

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnDieEvent();
	virtual void OnPlayLevelSequence();
	UFUNCTION(BlueprintCallable)
	virtual void OnEndLevelSequence();

	void OnPatternCompleted(EBossStateType CompletedPattern);
	EBossStateType SelectPatternByWeight(EBossStateType PreviousPattern);
	void StartPattern(EBossStateType NextPattern);
	void ResetAttackCount();

	// 캐릭터 바라보기 제어 함수들
	void EnableLookAtTarget();
	void DisableLookAtTarget();

	// 애니메이션 노티파이 이벤트 처리
	UFUNCTION()
	void OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION(BlueprintCallable)
	float GetTargetDistance();

	UFUNCTION(BlueprintCallable)
	void ApplyDamageToTarget(TSubclassOf<UGameplayEffect> EffectClass , AActor* ToTaget);


private:
	UPROPERTY()
	TObjectPtr<AAIController> AIController;
	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ADKCharacterBase> Target = nullptr;

	UPROPERTY()
	TObjectPtr<UBossStateBase> CurrentStateBase;

	UPROPERTY()
	TMap<EBossStateType, TObjectPtr<UBossStateBase>> StateMap;

	bool bIsInitialized = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDKPlayWidget> BossHealth_Class;

	UPROPERTY(EditDefaultsOnly)
	class ULevelSequence* BossSequence;

	UPROPERTY()
	TObjectPtr<UDKPlayWidget> healthBar;
private:
	void SetBossData();
	void SetBossInit();


	UBossStateBase* GetOrCreateState(EBossStateType BossState);
	bool IsHaveState(EBossStateType BossState);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// GameplayEffect 적용 감지를 위한 델리게이트 바인딩
	UFUNCTION()
	void OnGameplayEffectApplied(UAbilitySystemComponent* TargetAbility, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

	void OnHPChanged(const FOnAttributeChangeData& Data);




};
