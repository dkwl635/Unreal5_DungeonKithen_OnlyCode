// Copyright © 2025 Tartare Studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * DKGameplayTags
 *
 * Singleton containing native Gameplay Tags
 */

struct FDKGameplayTags
{
public:
	static const FDKGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	FGameplayTag Attributes_Primary_DEF;
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_CriticalRate;

	FGameplayTag Damage;
protected:

private:
	static FDKGameplayTags GameplayTags;
};
