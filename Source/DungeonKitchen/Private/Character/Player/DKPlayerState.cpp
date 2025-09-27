// Copyright © 2025 Tartare Studio


#include "Character/Player/DKPlayerState.h"

#include "AbilitySystem/DKAbilitySystemComponent.h"
#include "AbilitySystem/DKCharacterAttributeSet.h"

ADKPlayerState::ADKPlayerState()
{
	//NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UDKAbilitySystemComponent>("AbilitySystemComponent");
	//AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UDKCharacterAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* ADKPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
