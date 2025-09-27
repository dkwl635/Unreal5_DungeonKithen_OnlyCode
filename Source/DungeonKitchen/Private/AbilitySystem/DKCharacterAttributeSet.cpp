// Copyright © 2025 Tartare Studio


#include "AbilitySystem/DKCharacterAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Game/DKPlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


UDKCharacterAttributeSet::UDKCharacterAttributeSet()
{
	InitHP(100.f);

	InitATKPct(1.0f);
	InitDEFPct(0.0f);
	InitMoveSpeedPct(1.0f);
}

void UDKCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& Input)
{
	Super::PreAttributeChange(Attribute, Input);

}

void UDKCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	// 어떤 Attribute가 변경되었는지
	FGameplayAttribute ChangedAttribute = Data.EvaluatedData.Attribute;

	// 실제 변경된 값 (최종 적용 값)
	float NewValue = Data.EvaluatedData.Magnitude;

	OnChangeValueAttributes.Broadcast(ChangedAttribute, NewValue);


	if (ChangedAttribute== GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0, GetMaxHP()));
	}

	if (ChangedAttribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0);
		if (LocalIncomingDamage > 0)
		{
			const float NewHP = GetHP() - LocalIncomingDamage;
			SetHP(FMath::Clamp(NewHP, 0, GetMaxHP()));

			const bool bFatal = NewHP <= 0.f;
			if(Props.TargetCharacter == UGameplayStatics::GetPlayerCharacter(GetWorld(),0))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Player Hit Attribute"));
			}
			else
			{
				ShowFloatingText(Props, LocalIncomingDamage);
			}
		}
	}
}

void UDKCharacterAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage) const
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		if (ADKPlayerController* PC = Cast<ADKPlayerController>(UGameplayStatics::GetPlayerController(Props.SourceCharacter, 0)))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter);
		}
	}
}


void UDKCharacterAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}
