// Copyright © 2025 Tartare Studio


#include "Game/Subsystem/DigestiveSystem.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Actor/Weapon/WeaponBase.h"
#include "Data/EffectDataStructures.h"
#include "Data/ItemDataStructures.h"
#include "Character/Player/DKPlayerCharacter.h"
#include "DishEffect/DishEffectBase.h"
#include "Kismet/GameplayStatics.h"

void UDigestiveSystem::SetPlayerCharacter(AActor* Actor)
{
	FindCharacter();
}

void UDigestiveSystem::AddDish(UDKDish* Dish)
{
	//중복 방지
	if (Dishes.Contains(Dish))
	return;

	FindCharacter();
	Dishes.Add(Dish);

	//단순 값 증가 처리
	if (Dish->EffectData->ConditionTag.MatchesTag(FGameplayTag::RequestGameplayTag("EffectCondition.Always")))
	{
		AddCharacterEffect(Dish,PlayerCharacter.Get());
	}
	else
	{
		if (!TriggerDishes.Contains(Dish->EffectData->ConditionTag))
			TriggerDishes.Add(Dish->EffectData->ConditionTag, TArray<UDKDish*>());

		TriggerDishes[Dish->EffectData->ConditionTag].Add(Dish);
	}
	//특수 조건 효과 처리

	OnRefresh.Broadcast();
}

void UDigestiveSystem::RemoveDish(UDKDish* Dish)
{
	FindCharacter();
	if (!Dishes.Contains(Dish))
		return;


	Dishes.Remove(Dish);

	//단순 값 감소 처리
	if (Dish->EffectData->ConditionTag.MatchesTag(FGameplayTag::RequestGameplayTag("EffectCondition.Always")))
	{
		RemoveCharacterEffect(Dish,PlayerCharacter.Get());
	}
	else
	{
		if (TriggerDishes.Contains(Dish->EffectData->ConditionTag) && 	TriggerDishes[Dish->EffectData->ConditionTag].Contains(Dish))
		{
			TriggerDishes[Dish->EffectData->ConditionTag].Remove(Dish);
		}
	}

	OnRefresh.Broadcast();
}

void UDigestiveSystem::StartTriggerDishes(FString Condition, ADKCharacterBase* Target)
{
	// "EffectCondition.%s" 포맷 문자열로 최종 태그 문자열 생성
	FString TagString = FString::Printf(TEXT("EffectCondition.%s"), *Condition);

	// FString → FName → GameplayTag
	FGameplayTag ConditionTag = FGameplayTag::RequestGameplayTag(FName(*TagString));

	if (!TriggerDishes.Contains(ConditionTag) || TriggerDishes[ConditionTag].IsEmpty())
		return;

	for (UDKDish* Dish : TriggerDishes[ConditionTag])
	{
		//확률 및 조건 체크
		if (!CheckTriggerDishes(Dish))
			continue;


		if (PlayerTargetTag.HasTag(Dish->EffectData->EffectTarget1))
		{
			Target = PlayerCharacter.Get();
		}

		//특수 이펙트 처리
		if (CheckSpicalDishEffect(Dish, Target))
			return;


		//이미 적용 중인 이펙트 가 있는지
		if (CheckTargetDishEffect(Dish, Target))
			continue;

		AddCharacterEffect(Dish, Target);
	}
}

void UDigestiveSystem::FindCharacter()
{
	if (PlayerCharacter.IsValid())
		return;

	if (ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer())
		if (LP)
		{
			auto* pc = LP->GetWorld()->GetFirstPlayerController();
			if (pc)
			{
				PlayerCharacter = pc->GetPawn<ADKPlayerCharacter>();

			}
		}
}





void UDigestiveSystem::AddCharacterEffect(UDKDish* Dish, ADKCharacterBase* Target)
{
	UDishEffectBase* EffectBase = NewObject<UDishEffectBase>();
	EffectBase->SetEffect(Dish);

	Target->AddDishEffect(EffectBase);
}

void UDigestiveSystem::RemoveCharacterEffect(UDKDish* Dish , ADKCharacterBase* Target)
{
	Target->RemoveDishEffect(Dish);
}

bool UDigestiveSystem::CheckTriggerDishes(UDKDish* Dish)
{
	//확률 체크
	if (RateConditionTag.HasTag(Dish->EffectData->ConditionTag))
	{
		int32 Rate = Dish->EffectData->ConditionValue; // 0 ~ 100
		int32 Random = FMath::RandRange(1, 100);

		return  Random <= Rate;
	}
	//Kill 횟수
	else if (KillCountConditionTag.HasTag(Dish->EffectData->ConditionTag))
	{
		Dish->TempInt++;
		if (Dish->TempInt == Dish->EffectData->ConditionValue)
		{
			Dish->TempInt = 0;
			return true;
		}
	}

	return false;




}

bool UDigestiveSystem::CheckTargetDishEffect(UDKDish* Dish, ADKCharacterBase* Target) const
{
	return  Target->CheckDishEffect(Dish);
}

bool UDigestiveSystem::CheckSpicalDishEffect(UDKDish* Dish, ADKCharacterBase* Target) const
{
	FGameplayTag SpicalTag = Dish->EffectData->EffectTarget1;
	if (SpicalTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("EffectTarget.Explosion"))))
	{
		if (ExplosionActorClass)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(Target->GetActorLocation());
			SpawnTransform.SetRotation(FQuat(FRotator(0,0,0)));

			AActor* Boom =  Target->GetWorld()->SpawnActorDeferred<AActor>(ExplosionActorClass,SpawnTransform,
				nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

			UGameplayStatics::FinishSpawningActor(Boom, SpawnTransform);
			return true;
		}
	}

	return  false;
}

void UDigestiveSystem::PrepareForRestart()
{
	bWipeOnNextLoad = true;
	SavedDishBlocks.Empty();

	Dishes.Empty();
	TriggerDishes.Empty();
}
