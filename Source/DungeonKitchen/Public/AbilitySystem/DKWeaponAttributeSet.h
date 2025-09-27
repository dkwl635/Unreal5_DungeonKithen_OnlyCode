// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DKWeaponAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeValueWeaponAttributeSet, float , Value );
/**
 *
 */
UCLASS()
class DUNGEONKITCHEN_API UDKWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDKWeaponAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, Damage);

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData FireDelay;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, FireDelay);

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData ReloadTime;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, ReloadTime);

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData Rounds;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, Rounds);

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, Ammo);

	UPROPERTY(BlueprintReadOnly, Category="Weapon Attributes")
	FGameplayAttributeData CurAmmo;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, CurAmmo);

	// 보조속성
	UPROPERTY(BlueprintReadOnly, Category="Weapon Sub Attributes")
	FGameplayAttributeData FinalDamage;
	ATTRIBUTE_ACCESSORS(UDKWeaponAttributeSet, FinalDamage);

	UPROPERTY(BlueprintAssignable)
	FOnChangeValueWeaponAttributeSet OnChangeValueWeaponAttributes;
};
