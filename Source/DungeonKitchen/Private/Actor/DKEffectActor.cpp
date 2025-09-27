// Copyright © 2025 Tartare Studio


#include "Actor/DKEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DKGameplayTags.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"
#include "Actor/Weapon/WeaponBase.h"
#include "Character/Player/DKPlayerCharacter.h"

ADKEffectActor::ADKEffectActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void ADKEffectActor::BeginPlay()
{
	Super::BeginPlay();

}

void ADKEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	ADKPlayerCharacter* PlayerCharacter = Cast<ADKPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	UAbilitySystemComponent* PlayerASC = PlayerCharacter->GetAbilitySystemComponent();
	const UDKCharacterAttributeSet* AttributeSet = Cast<UDKCharacterAttributeSet>(PlayerASC->GetAttributeSet(UDKCharacterAttributeSet::StaticClass()));

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = PlayerASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(PlayerCharacter);
	FGameplayEffectSpecHandle EffectSpecHandle = PlayerASC->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContextHandle);

	FDKGameplayTags GameplayTags = FDKGameplayTags::Get();
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Damage, AttributeSet->GetWeaponDamage());

	PlayerASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),TargetASC);
}


