// Copyright © 2025 Tartare Studio


#include "UI/WidgetController/DKOverlayWidgetController.h"

#include "AbilitySystem/DKCharacterAttributeSet.h"
#include <AbilitySystem/DKAbilitySystemComponent.h>

#include "AbilitySystem/DKWeaponAttributeSet.h"
#include "Actor/Weapon/WeaponBase.h"
#include "Character/Player/DKPlayerCharacter.h"

void UDKOverlayWidgetController::BroadcastInitialValues()
{
	UDKCharacterAttributeSet* DKAttributeSet = CastChecked<UDKCharacterAttributeSet>(AttributeSet);

	OnHPChanged.Broadcast(DKAttributeSet->GetHP());
	OnMaxHPChanged.Broadcast(DKAttributeSet->GetMaxHP());

}

void UDKOverlayWidgetController::BindCallbacksToDependencies()
{
	const UDKCharacterAttributeSet* DKAttributeSet = CastChecked<UDKCharacterAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		DKAttributeSet->GetHPAttribute()).AddUObject(this, &UDKOverlayWidgetController::HPChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		DKAttributeSet->GetMaxHPAttribute()).AddUObject(this, &UDKOverlayWidgetController::MaxHPChanged);

	Cast<UDKAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
	[](const FGameplayTagContainer& AssetTags)
	{
		for (const FGameplayTag& Tag : AssetTags)
		{
			const FString Msg = FString::Printf(TEXT("GE Tag : %s"), *Tag.ToString());
			//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, Msg);
		}
	});

}

void UDKOverlayWidgetController::HPChanged(const FOnAttributeChangeData& Data) const
{
	OnHPChanged.Broadcast(Data.NewValue);
}

void UDKOverlayWidgetController::MaxHPChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHPChanged.Broadcast(Data.NewValue);
}
