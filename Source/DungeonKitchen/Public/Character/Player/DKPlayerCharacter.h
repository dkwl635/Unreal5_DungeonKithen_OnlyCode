// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "Character/DKCharacterBase.h"
#include "DKPlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDieEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeWeaponEvent, AWeaponBase*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeCheckWeaponEvent, AActor*, CheckActor);

class AWeaponBase;

UENUM(BlueprintType)
enum class EDKPlayerState : uint8
{
	Idle,
	Reloading,
};

/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API ADKPlayerCharacter : public ADKCharacterBase
{
	GENERATED_BODY()
public:
	ADKPlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void InitAbilityActorInfo() override;

	void Attack();

	void OnAttackReleased();

	EDKPlayerState GetCurPlayerState();

	void SetPlayerState(EDKPlayerState NewState);

	void EquipWeapon(AWeaponBase* NewWeapon);

	void UnEquipWeapon();

	void ChangeWeapon();

	void OnDeath();

	UPROPERTY(Blueprintreadwrite)
	TObjectPtr<class AWeaponBase> Weapon;

	UPROPERTY()
	TObjectPtr<class AWeaponBase> Weapon1;

	UPROPERTY()
	TObjectPtr<class AWeaponBase> Weapon2;

	bool bIsWeapon1 = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> WeaponPos;

	bool bCanAttack = true;

	float FireDelay=0.5;

	UPROPERTY(EditAnywhere)
	float FindDistance = 500;

	AActor* CheckActor = nullptr;

	FOnPlayerDieEvent OnPlayerDieEvent;

	UPROPERTY(BlueprintAssignable)
	FOnChangeWeaponEvent OnChangeWeaponEvent;

	UPROPERTY(BlueprintAssignable)
	FOnChangeCheckWeaponEvent OnChangeCheckWeaponEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SceneSpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class USceneCaptureComponent2D> SceneCapture;

protected:
	virtual void BeginPlay() override;

	void OnHPAttributeChanged(const struct FOnAttributeChangeData& Data);

	void checkTrace();

	EDKPlayerState PlayerState = EDKPlayerState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class AWeaponBase> WeaponClass;

	virtual void ApplyEffectToWeapon(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const override;

	float BaseMoveSpeed = 850.f;
};
