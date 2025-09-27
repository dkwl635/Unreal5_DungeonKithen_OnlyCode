// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DKCharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties(){}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChangeValueAttributeSet,const FGameplayAttribute&, GameplayAttributeData, float , Value );
UCLASS()
class DUNGEONKITCHEN_API UDKCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UDKCharacterAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& Input) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category="Vital Attributes")
	FGameplayAttributeData HP;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, HP);

	UPROPERTY(BlueprintReadOnly, Category="Vital Attributes")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, MaxHP);

	//플레이어는 무기의 공격력을 사용 , 몬스터는 ATK Damge로 사용
	//플레이어는 필요없으나 몬스터는 필요함
	UPROPERTY(BlueprintReadOnly, Category="Primary Attributes")
	FGameplayAttributeData ATK;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, ATK);

	//사용 X
	UPROPERTY(BlueprintReadOnly, Category="Primary Attributes")
	FGameplayAttributeData DEF;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, DEF);


	// Secondary

	//사용 X
	UPROPERTY(BlueprintReadOnly, Category="Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, Armor);

	//치명타 확률
	UPROPERTY(BlueprintReadOnly, Category="Secondary Attributes")
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, CriticalRate);
	//사용 X
	UPROPERTY(BlueprintReadOnly, Category="Secondary Attributes")
	FGameplayAttributeData WeaponDamage;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, WeaponDamage);

	// meta attributes

	//애매
	UPROPERTY(BlueprintReadOnly, Category="Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, IncomingDamage);


	//추가 능력치 값
	UPROPERTY(BlueprintReadOnly, Category="Add Attributes")
	FGameplayAttributeData AddATK;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, AddATK);

	//능력치 퍼센트 배율 값
	UPROPERTY(BlueprintReadOnly, Category="Pct Attributes")
	FGameplayAttributeData ATKPct;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, ATKPct);

	UPROPERTY(BlueprintReadOnly, Category="Pct Attributes")
	FGameplayAttributeData DEFPct;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, DEFPct);

	UPROPERTY(BlueprintReadOnly, Category="Pct Attributes")
	FGameplayAttributeData MoveSpeedPct;
	ATTRIBUTE_ACCESSORS(UDKCharacterAttributeSet, MoveSpeedPct);





	UPROPERTY(BlueprintAssignable, Category="Meta Attributes")
	FOnChangeValueAttributeSet OnChangeValueAttributes;
private:
	void SetEffectProperties(const FGameplayEffectModCallbackData &Data, FEffectProperties& Props) const;

	void ShowFloatingText(const FEffectProperties& Props, float Damage) const;
};

