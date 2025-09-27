// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"

#include "DishEffectBase.generated.h"

/**
 *
 */


UCLASS(Blueprintable)
class DUNGEONKITCHEN_API UDishEffectBase : public UObject
{
	GENERATED_BODY()

public:
	UDishEffectBase();

	UPROPERTY()
	class UDKDish* EffectMakerDish;

	UPROPERTY()
	class ADKCharacterBase* TargetCharacter;


	float Timer = -1;
	//true == 이펙트 종료
	virtual bool TickEffect(float DeltaTime);
	virtual void SetEffect(UDKDish* Dish);
	virtual void StartEffect(ADKCharacterBase* Target );
	virtual void EndEffect();
	virtual void ResetEffect();
	/*단순 값 변경 조건 */
	void SetAttributeValue(UDKDish* Dish, bool bIsEffect);
	void SetAttributeValue(const FGameplayTag TargetEffect , float Value, bool bIsEffect);
	/*단순 값 변경 조건 */
	FGameplayAttribute GetTargetAttribute(FGameplayTag EffectTarget) ;


	/*특정 값 처리*/
	void ApplyAbilityEffect(const FGameplayAttribute& Attr , float Value1);
	void RemoveAbilityEffect(const FGameplayAttribute& Attr , float Value1);
	/*특정 값 처리*/

};
