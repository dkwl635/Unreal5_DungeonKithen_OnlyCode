// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Character/DKCharacterBase.h"
#include "Data/EnemyDataStructures.h"
#include "GameplayEffectTypes.h"
#include "DKEnemy.generated.h"



UENUM(BlueprintType)
enum class EDKEnemyState : uint8
{
	Spawning UMETA(DisplayName = "Spawning"),
	Idle  UMETA(DisplayName = "Idle"),
	Attack UMETA(DisplayName = "Attack"),
	Death  UMETA(DisplayName = "Death"),
	Trace  UMETA(DisplayName = "Trace"),
	Hit   UMETA(DisplayName = "Hit"),
	AttackAfterMove UMETA(DisplayName = "AttackAtfterMove"),
	NONE UMETA(DisplayName = "None"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangeEnemy, float , Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDKEnemyEvent, AActor* , Enemy);


UCLASS()
class DUNGEONKITCHEN_API ADKEnemy : public ADKCharacterBase
{

	GENERATED_BODY()

public:
	static int32 EnemyLevel;
	UFUNCTION(BlueprintCallable)
	static void EnemyLevelUp(){EnemyLevel++;}
public:
	ADKEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void InitEnemyData();
	void EnemySetting();

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeEnemy OnChangeHP;
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeEnemy OnChangeMaxHP;

	UFUNCTION(BlueprintCallable)
	class AEnemyAttacker* CreateEnemyAttacker(TSubclassOf<AEnemyAttacker> EnemyAttackerClass , FVector Location , FRotator Rotation );
public:
	UPROPERTY(EditAnywhere,Category = "Enemy|State")
	EDKEnemyState CurrentState = EDKEnemyState::NONE;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Enemy|Info")
	FName EnemyName = TEXT("EnemyName");
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Enemy|Info")
	int32 Level = 1;
	UPROPERTY(EditAnywhere,Category = "Enemy|Info")
	int32 EnemyID = -1;
	UPROPERTY()
	TObjectPtr<class AAIController> AIController;
	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;
	UFUNCTION(BlueprintCallable)
	class ADKPlayerCharacter* GetTarget() {return Target.Get();}
	UFUNCTION(BlueprintCallable)
	bool GetAlive() {return bIsAlive;}

	UFUNCTION(BlueprintCallable)
	void ApplyDamageToTarget(AActor* DamageTarget);
public:
	UPROPERTY(BlueprintAssignable)
	FOnDKEnemyEvent OnEnemyDieEvent;

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy|Anim")
	TMap<EDKEnemyState , TObjectPtr<UAnimMontage>> StateMontage;

protected:
	UPROPERTY(EditAnywhere,Category = "Enemy|HPBar")
	TObjectPtr<class UWidgetComponent> HealthBar;
	// 데미지용 GameplayEffect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	TSubclassOf<class UGameplayEffect> DamageGameplayEffect;
	UPROPERTY(EditAnywhere, Category = "Enemy|Attack")
	TSubclassOf<class UDKEnemyAttack> EnemyAttackClass;
	UPROPERTY(EditAnywhere, Category = "Enemy|Attack")
	float AttackDis = 300.f;


	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<class ADKPlayerCharacter> Target;

protected :
	/*셋팅 함수 들*/
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnAnimNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION()
	void OnGameplayEffectApplied(UAbilitySystemComponent* TargetAbility, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnMaxHPChanged(const FOnAttributeChangeData& Data);

	/*셋팅 함수 들*/
	void StartState(EDKEnemyState NewState);
	void FindTarget();

	void StartStateIdle();
	void StartStateTrace();
	void StartStateAttack();
	void StartStateHit();
	void StartStateDeath();

	void TickStateTrace(float DeltaTime);

private:
	bool bIsInitialized = false;
	bool bIsHit = false;
	bool bIsAlive = false;
	FDKEnemyData EnemyData;

	UPROPERTY()
	TObjectPtr<class UDKEnemyAttack> EnemyAttack;

	float LastMovementUpdateTime = 0.0f;
	float MovementUpdateInterval = 0.3f;
private:
	void UpdatePathFollowing();
	// HealthBar 빌보드 기능 업데이트
	void UpdateHealthBarBillboard();

	int32 GetSpawnItemID();

};
