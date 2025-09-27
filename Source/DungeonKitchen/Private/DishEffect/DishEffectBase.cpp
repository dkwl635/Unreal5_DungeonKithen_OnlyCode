// Copyright © 2025 Tartare Studio


#include "DishEffect/DishEffectBase.h"

#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Character/DKCharacterBase.h"
#include "Data/ItemDataStructures.h"

UDishEffectBase::UDishEffectBase()
{

}

bool UDishEffectBase::TickEffect(float DeltaTime)
{
	if (Timer > 0.0f)
	{
		Timer -= DeltaTime;
		if (Timer <= 0.0f)
		{
			EndEffect();

			return true;
		}
	}

	return false;

}

void UDishEffectBase::SetEffect(UDKDish* Dish)
{
	EffectMakerDish = Dish;
}

void UDishEffectBase::StartEffect(ADKCharacterBase* Target)
{
	if (!EffectMakerDish)
		return;
	TargetCharacter = Target;

	FDKDishEffectData* Data = EffectMakerDish->EffectData;
	if (!Data)
		return;

	if (Data->Time > 0.0f)
	{
		Timer = Data->Time;
	}
	else
	{
		Timer = -1.0f;
	}


	SetAttributeValue(EffectMakerDish, true);


}


void UDishEffectBase::EndEffect()
{
	SetAttributeValue(EffectMakerDish , false);
}

void UDishEffectBase::ResetEffect()
{
	Timer =  EffectMakerDish->EffectData->Time;
}

void UDishEffectBase::SetAttributeValue(UDKDish* Dish, bool bIsEffect)
{
	const FGameplayTag EffectTarget = Dish->EffectData->EffectTarget1;
	float Value = Dish->EffectData->EffectValue1;


	const FGameplayTag EffectTarget2 = Dish->EffectData->EffectTarget2;
	float Value2 = Dish->EffectData->EffectValue2;

	//효과 적용 시키기
	SetAttributeValue(EffectTarget, Value ,bIsEffect);
	SetAttributeValue(EffectTarget2, Value2, bIsEffect);
	Dish->bIsEffect = bIsEffect;
}

void UDishEffectBase::SetAttributeValue(const FGameplayTag TargetEffect, float Value, bool bIsEffect)
{
	FGameplayAttribute TargetAttribute = GetTargetAttribute(TargetEffect);

	if (!TargetAttribute.IsValid())
		return;

	if (bIsEffect)
	{
		ApplyAbilityEffect(TargetAttribute, Value);
	}
	else
	{
		RemoveAbilityEffect(TargetAttribute,Value);
	}
}

FGameplayAttribute UDishEffectBase::GetTargetAttribute(FGameplayTag EffectTarget)
{
	if (EffectTarget.MatchesTag(FGameplayTag::RequestGameplayTag("EffectTarget.AddMaxHealth")))
	{
		return UDKCharacterAttributeSet::GetMaxHPAttribute();
	}
	else if (EffectTarget.MatchesTag(FGameplayTag::RequestGameplayTag("EffectTarget.AddAttack")))
	{
		return  UDKCharacterAttributeSet::GetAddATKAttribute();
	}
	else if (EffectTarget.MatchesTag(FGameplayTag::RequestGameplayTag("EffectTarget.AddAttackPct")))
	{
		return  UDKCharacterAttributeSet::GetATKPctAttribute();
	}
	else if (EffectTarget.MatchesTag(FGameplayTag::RequestGameplayTag("EffectTarget.AddDefencePct")))
	{
		return UDKCharacterAttributeSet::GetDEFPctAttribute();
	}
	else if (EffectTarget.MatchesTag(FGameplayTag::RequestGameplayTag("EffectTarget.AddMoveSpeedPct")))
	{
		return UDKCharacterAttributeSet::GetMoveSpeedPctAttribute();
	}
	return FGameplayAttribute();
}

void UDishEffectBase::ApplyAbilityEffect(const FGameplayAttribute& Attr, float Value1)
{
	UAbilitySystemComponent* TargetGAS = TargetCharacter->GetAbilitySystemComponent();
	const float Before = TargetGAS->GetNumericAttribute(Attr);
	const float Result =  Before + Value1;

	TargetGAS->SetNumericAttributeBase(Attr, Result);

	const float After = TargetGAS->GetNumericAttribute(Attr);

	UE_LOG(LogTemp, Warning, TEXT("[Digest] +%s ,  %s: %.2f -> %.2f  (∆=%.2f)"),
		*Attr.AttributeName,
		*Attr.GetName(), Before, After, Value1);


	auto* AttributeSet = Cast<UDKCharacterAttributeSet>(TargetGAS->GetAttributeSet(UDKCharacterAttributeSet::StaticClass()));
	AttributeSet->OnChangeValueAttributes.Broadcast(Attr,After);
}

void UDishEffectBase::RemoveAbilityEffect(const FGameplayAttribute& Attr, float Value1)
{
	UAbilitySystemComponent* TargetGAS = TargetCharacter->GetAbilitySystemComponent();
	const float Before = TargetGAS->GetNumericAttribute(Attr);
	const float Result =  Before - Value1;

	TargetGAS->SetNumericAttributeBase(Attr, Result);

	const float After = TargetGAS->GetNumericAttribute(Attr);

	UE_LOG(LogTemp, Warning, TEXT("[Digest] +%s ,  %s: %.2f -> %.2f  (∆=%.2f)"),
		*Attr.AttributeName,
		*Attr.GetName(), Before, After, Value1);


	auto* AttributeSet = Cast<UDKCharacterAttributeSet>(TargetGAS->GetAttributeSet(UDKCharacterAttributeSet::StaticClass()));
	AttributeSet->OnChangeValueAttributes.Broadcast(Attr,After);
}
