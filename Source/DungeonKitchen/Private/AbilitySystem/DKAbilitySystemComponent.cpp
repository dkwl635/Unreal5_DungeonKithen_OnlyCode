// Copyright © 2025 Tartare Studio


#include "AbilitySystem/DKAbilitySystemComponent.h"

#include "DKGameplayTags.h"

void UDKAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UDKAbilitySystemComponent::EffectApplied);

	const FDKGameplayTags& GameplayTags = FDKGameplayTags::Get();
	//GameplayTags.Attributes_Secondary_Armor.ToString()
}

void UDKAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
                                              const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);

}
