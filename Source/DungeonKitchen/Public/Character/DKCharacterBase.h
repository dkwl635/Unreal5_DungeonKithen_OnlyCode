// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "DKCharacterBase.generated.h"

struct FGameplayAttribute;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class DUNGEONKITCHEN_API ADKCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADKCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UPaperSpriteComponent> MinimapSprite;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> EffectWeaponAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	virtual void ApplyEffectToWeapon(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	void InitializeDefaultAttributes() const;

	UFUNCTION()
	virtual void SetChangeAttribute(const FGameplayAttribute& GameplayAttributeData, float  Value);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> DashSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> WalkSound;

	//FTimerHandle WalkCheckTimer;
	//FTimerHandle WalkTimer;

	float CurSoundTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float WalkSoundDelay = 0.5f;

	virtual void PlayWalkSound();

	void EffectDishTick(float DeltaSeconds);
	void AddDishEffect(class UDishEffectBase* DishEffect);
	void RemoveDishEffect(class UDKDish* TargetDish);
	//이미 적용중인 Effect 가 있는지 체크 true 이미 적용중인 효과 있음
	bool CheckDishEffect(class UDKDish* TargetDish) const;
	UPROPERTY()
	TArray<class UDishEffectBase*> DishEffects;


	protected:
	float InitMoveSpeed = 0;
};
