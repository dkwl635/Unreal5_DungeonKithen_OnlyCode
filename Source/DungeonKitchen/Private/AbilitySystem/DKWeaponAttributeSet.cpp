// Copyright © 2025 Tartare Studio


#include "AbilitySystem/DKWeaponAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"

UDKWeaponAttributeSet::UDKWeaponAttributeSet()
{
	// 기본 무기 속성값 설정 - GameplayEffect Modifier가 올바르게 작동하도록
	//InitDamage(10.0f);
	//InitFireDelay(0.5f);
	//InitReloadTime(2.0f);
}

void UDKWeaponAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;

	// 어떤 Attribute가 변경되었는지
	FGameplayAttribute ChangedAttribute = Data.EvaluatedData.Attribute;

	// 실제 변경된 값 (최종 적용 값)
	float NewValue = Data.EvaluatedData.Magnitude;

	//OnChangeValueWeaponAttributes.Broadcast(NewValue);
}
