// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Data/ItemDataStructures.h"
#include "GameFramework/Actor.h"
#include "Interface/DKInteractInterface.h"
#include "WeaponBase.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;
class UBoxComponent;

UCLASS()
class DUNGEONKITCHEN_API AWeaponBase : public AActor, public IAbilitySystemInterface, public IDKInteractInterface
{
	GENERATED_BODY()

public:
	AWeaponBase();

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USceneComponent> SceneComp;

	UPROPERTY(EditAnywhere, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> WeaponBox;

	virtual void Attack();

	virtual void OnAttackCompleted();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const;

	virtual void Interact() override;

	UPROPERTY(EditDefaultsOnly)
	int32 WeaponID = -1;

	FDKWeaponData WeaponData;

	UPROPERTY(EditDefaultsOnly)
	float MaxAmmo = 10;
	UPROPERTY(EditDefaultsOnly)
	float CurAmmo = MaxAmmo;

	void SetCurAmmo();
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantDamageGameplayEffectClass;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultWeaponAttributes;

	void InitDefaultWeaponAttributes();

	void SetWeaponData();

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> AttackSound;
};
