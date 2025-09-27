// Copyright © 2025 Tartare Studio


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "DKGameplayTags.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "Game/Subsystem/DigestiveSystem.h"

struct DKDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AddATK);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ATKPct);

	DECLARE_ATTRIBUTE_CAPTUREDEF(DEFPct);

	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalRate);
	DKDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDKCharacterAttributeSet, AddATK, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UDKCharacterAttributeSet, ATKPct, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UDKCharacterAttributeSet, DEFPct, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UDKCharacterAttributeSet, CriticalRate, Source, false);
	}
};

static const DKDamageStatics& DamageStatics()
{
	static DKDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AddATKDef);
	RelevantAttributesToCapture.Add(DamageStatics().ATKPctDef);
	RelevantAttributesToCapture.Add(DamageStatics().DEFPctDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalRateDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	/*음식 효과체크 하기*/
	//우선 플레이어 캐릭터만 체크
	if (Cast<ADKPlayerCharacter>(SourceAvatar))
	{
		if (UDigestiveSystem* DigestiveSystem = SourceASC->GetWorld()->GetGameInstance()->GetSubsystem<UDigestiveSystem>())
		{
			DigestiveSystem->StartTriggerDishes(TEXT("OnHit"), Cast<ADKCharacterBase>(SourceAvatar));
		}
	}
	/*음식 효과체크 하기*/

	// Get Damage Set by Caller Magnitude
	float Damage = Spec.GetSetByCallerMagnitude(FDKGameplayTags::Get().Damage);

	float AddATK = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AddATKDef, EvaluateParameters, AddATK);
	AddATK = FMath::Max(0, AddATK);

	float ATKPct = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ATKPctDef, EvaluateParameters, ATKPct);
	ATKPct = FMath::Max(0.1f, ATKPct);

	//1차 데미지 계산
	Damage = (Damage + AddATK) * ATKPct;


	float SourceCriticalRate = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalRateDef, EvaluateParameters, SourceCriticalRate);
	SourceCriticalRate = FMath::Max<float>(0, SourceCriticalRate);

	// critical
	const bool bCriticalHit = FMath::RandRange(1, 100) < SourceCriticalRate;
	Damage = bCriticalHit ? 2.f * Damage : Damage;


	//방어 감소 율
	float DefPct = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DEFPctDef, EvaluateParameters, DefPct);



	// 최종 입히는 데미지 - 소수점은 그냥 버림
	Damage = FMath::TruncToInt( Damage - (Damage * DefPct * 0.01f));


	const FGameplayModifierEvaluatedData EvaluatedData(UDKCharacterAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);

	if(ADKPlayerCharacter* Player = Cast<ADKPlayerCharacter>(TargetAvatar))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Hit Calc"));
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*EnemyDamage*/

UExecCalc_EnemyDamage::UExecCalc_EnemyDamage()
{
	RelevantAttributesToCapture.Add(DamageStatics().AddATKDef);
	RelevantAttributesToCapture.Add(DamageStatics().ATKPctDef);
	RelevantAttributesToCapture.Add(DamageStatics().DEFPctDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalRateDef);
}

void UExecCalc_EnemyDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// Get Damage Set by Caller Magnitude
	float Damage = Spec.GetSetByCallerMagnitude(FDKGameplayTags::Get().Damage);

	float AddATK = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AddATKDef, EvaluateParameters, AddATK);
	AddATK = FMath::Max(0, AddATK);

	float ATKPct = 1.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ATKPctDef, EvaluateParameters, ATKPct);
	ATKPct = FMath::Max(0.1f, ATKPct);

	//1차 데미지 계산
	Damage = (Damage + AddATK) * ATKPct;


	float SourceCriticalRate = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalRateDef, EvaluateParameters, SourceCriticalRate);
	SourceCriticalRate = FMath::Max<float>(0, SourceCriticalRate);

	// critical
	const bool bCriticalHit = FMath::RandRange(1, 100) < SourceCriticalRate;
	Damage = bCriticalHit ? 2.f * Damage : Damage;


	//방어 감소 율
	float DefPct = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DEFPctDef, EvaluateParameters, DefPct);



	// 최종 입히는 데미지 - 소수점은 그냥 버림
	Damage = FMath::TruncToInt( Damage - (Damage * DefPct));


	const FGameplayModifierEvaluatedData EvaluatedData(UDKCharacterAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
